#ifndef __HTTPCLIENTEXCHANGEINFO_HPP
#define __HTTPCLIENTEXCHANGEINFO_HPP

#include <map>
#include <string>
#include <mutex>
#include <pqxx/pqxx>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Types.hpp"
#include "Options.hpp"
#include "HttpClientBase.hpp"

namespace pt = boost::property_tree;
using namespace std;

class HttpClientExchangeInfo : public HttpClientBase {
    map<string, pair<string,string>>& active_symbols_;
    pqxx::connection csql_;
    
    void closure_action(bool fault_occured) {
        if(fault_occured) {
            LOG(warning) << "HttpClientExchangeInfo: failed!";
            return;
        }

	auto& res = res_parser_.get();
        stringstream ss(res.body());
        pt::ptree pt;
        pt::read_json(ss, pt);

        if(pt.count("symbols") == 0) {
            LOG(fatal) << "HttpClientExchangeInfo: JSON parse error!" << res.body();
	    assert(false);
            return;
        }

	pqxx::work w(csql_);
        for(auto& srec: pt.get_child("symbols")) {
	    const string symbol = srec.second.get<string>("symbol");
	    const string status = srec.second.get<string>("status");
	    const string base_asset = srec.second.get<string>("baseAsset");
	    const string quote_asset = srec.second.get<string>("quoteAsset");

	    if(status == "TRADING" &&
	       regex_search(symbol, regex("BTC|BNB|SOL|ETH|TRY|XRP"))) {
		active_symbols_[boost::algorithm::to_lower_copy(symbol)]
		    = make_pair(base_asset, quote_asset);

		w.exec(pqxx::prepped{"insert"},
		       pqxx::params(boost::algorithm::to_lower_copy(symbol)
				    , base_asset, quote_asset));
	    }
        }
	w.commit();

        { LOG(trace) << "HttpClientExchangeInfo: found " << active_symbols_.size() << " active symbols"; }

        success_ = true;
    }
public:
    void fetch() {
        get(options.val<string>("binance.api"), "/api/v3/exchangeInfo");        
    }

    HttpClientExchangeInfo(net::io_context &ioc, ssl::context &ctx,
			   map<string, pair<string, string>>& active_symbols)
    : HttpClientBase(ioc, ctx),
      active_symbols_(active_symbols),
      csql_(options.val<string>("psql"))
    {

	csql_.prepare(
	    "insert",
                    "INSERT INTO pairs (symbol, base, quote) VALUES ($1, $2, $3) ON CONFLICT (symbol) DO NOTHING"
	    );	
    }
        
};


#endif
