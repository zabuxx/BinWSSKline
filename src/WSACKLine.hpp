#ifndef WSACKLINE_HPP_
#define WSACKLINE_HPP_

#include <time.h>
#include <map>
#include <string>
#include <pqxx/pqxx>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>


#include "WSBase.hpp"
#include "Options.hpp"

namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;
namespace pt = boost::property_tree;

class WSACKline : public WSBase {
    const std::map<std::string, std::pair<std::string, std::string>>& active_symbols_;
    pqxx::connection csql_;
    
    
    void process_data(const std::string &buf) {
      std::stringstream ss(buf);
      pt::ptree pt_data;
      pt::read_json(ss, pt_data);

      //{ LOG(trace) << buf; }

      if(pt_data.count("data") == 0) {
        LOG(warning) << "WSACKline: Unrecognized message format: " << buf;
        return;
      }
      
      const auto& pt = pt_data.get_child("data");
            
      if (pt.count("e") == 0 || pt.get<std::string>("e") != "kline") {
        LOG(warning) << "WSACKline: Unrecognized message format: " << buf;
        return;
      }
      
      const auto &k = pt.get_child("k");
      const std::string symbol =
          boost::algorithm::to_lower_copy(k.get<std::string>("s"));
      if (active_symbols_.find(symbol) != active_symbols_.end()) {
        const std::string close_price = k.get<std::string>("c");
        const std::string base_asset_volume = k.get<std::string>("v");
        const int trade_count = k.get<int>("n");

	const long kstart = k.get<long>("t");
	const long kend = k.get<long>("T");
	const std::string& closed = k.get<std::string>("x");

	{
	    LOG(trace) << "WSACKline: " << kstart << "-" << kend
		       << " / " << closed
		       << ", symbol: " << symbol
		       << ", close_price: " << close_price
		       << ", base_asset_volume: " << base_asset_volume
		       << ", trade_count: " << trade_count;
	}
	
	

      } else {
        LOG(warning) << "WSACKline: Active symbol not found: " << symbol;
      }
    }

public:
    WSACKline(net::io_context& ioc, 
	      ssl::context& ctx, 
	      const std::map<std::string, std::pair<std::string, std::string>>& active_symbols) :
	WSBase(ioc, ctx),
	active_symbols_(active_symbols),
	csql_(options.val<string>("psql")) 
	{
	    // csql_.prepare(
	    // 	"insert",
	    // 	"INSERT INTO kline(tstamp, symbol, close_price, base_asset_volume, trade_count) VALUES ($1, $2, $3, $4, $5)");
	}


    void run() {
	std::string cmd = "/stream?streams=";
	bool first = true;

	for(const auto& pair: active_symbols_) {
	    if(!first) cmd += "/";
	    first = false;
	    cmd += pair.first + "@kline_1s";
	}

	WSBase::run(options.val<string>("binance.wss"),options.val<string>("binance.wssport"),cmd);	
    }

};

#endif


