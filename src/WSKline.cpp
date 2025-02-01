#include <sstream>
#include <time.h>
#include <mutex>
#include <thread>
#include <regex>


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

#include "Types.hpp"
#include "Options.hpp"
#include "WSKline.hpp"

namespace pt = boost::property_tree;

WSKline::WSKline(net::io_context& ioc, 
                 ssl::context& ctx, 
                 const set<string>& active_symbols,
                 const map<string,curfloat>& symbols_price) :
      WSBase(ioc, ctx),
      active_symbols_(active_symbols),
      csql_(options.val<string>("psql")),
      last_second_(time(0)),
      counter_(0)
      
{
    csql_.prepare(
        "insert",
        "insert into kline(tstamp, symbol, open, min, max, close, mean) values ($1, $2, $3, $4, $5, $6, $7)");


    ostringstream l1;
    l1 << "WSKline:::WSKLine: ";
    for(auto const& [symbol, price]: symbols_price) {
        if(price == 0.0)
          continue;

        l1 << symbol << "(" << price << ") ";
        klinemap_[symbol] = SymbAcc();
        klinemap_[symbol](price);


    }
    { LOG(trace) << l1.str(); }

    last_second_ = time(0);
}

void WSKline::time_cycle_kline() {
    pqxx::work w(csql_);

    for(const auto& [symb, _]: klinemap_) {

        const curfloat open_price = extract_result<acc::tag::first >(klinemap_[symb]);
        const curfloat min_price = extract_result<acc::tag::min >(klinemap_[symb]);
        const curfloat max_price = extract_result<acc::tag::max >(klinemap_[symb]);
        const curfloat close_price = extract_result<acc::tag::last >(klinemap_[symb]);
	const curfloat mean_price = extract_result<acc::tag::mean >(klinemap_[symb]);

        w.exec_prepared("insert",
                        last_second_ - options.val<int>("kline.cycle_time") / 2,
                        symb,
                        open_price,
                        min_price,
                        max_price,
                        close_price,
			mean_price
	    );

        this->klinemap_[symb] = SymbAcc();
        this->klinemap_[symb](close_price);

        }

    w.commit();
}

void WSKline::process_data(const string& buf) {
    counter_++;

    if(last_second_ + (options.val<int>("kline.cycle_time") - 1) < time(0)) {
        time_cycle_kline();

       { LOG(trace) << "WSKline: cycle after " << counter_ << " updates (" << 1.0 * counter_ / options.val<int>("kline.cycle_time") << " updates/s)"; }

        last_second_ = time(0);
        counter_ = 0;
    }

    stringstream ss(buf);

    pt::ptree pt;
    pt::read_json(ss, pt);

    if(pt.count("code")) {
        LOG(warning) << "WSKline: recieved error: " << pt.get<string>("msg");
        return;
    } else if(pt.count("data") == 0 || pt.get_child("data").count("a") == 0 || pt.get_child("data").count("b") == 0 || pt.get_child("data").count("s") == 0) {
        LOG(warning) << "WSKline: unknown message format: " << buf;
        return;
    }

    const curfloat ask = stold(pt.get_child("data").get<string>("a"));
    const curfloat bid = stold(pt.get_child("data").get<string>("b"));

    const curfloat price = (ask + bid)/2;
    const string symbol = boost::algorithm::to_lower_copy(pt.get_child("data").get<string>("s"));

    const auto sit = find(active_symbols_.begin(), active_symbols_.end(), symbol);
    if(sit != active_symbols_.end()) {
        klinemap_[symbol](price);
    
    }
}

void WSKline::run() {
    string cmd = "/stream?streams=";

    bool first = true;

    for(const auto& pair: active_symbols_) {
	
	if(!first)
	    cmd += "/";
	else 
	    first = false;
	
	cmd += pair + "@bookTicker";
    }
    
    WSBase::run(options.val<string>("binance.wss"),options.val<string>("binance.wssport"),cmd);
}
