#ifndef __HTTPCLIENTEXCHANGEINFO_HPP
#define __HTTPCLIENTEXCHANGEINFO_HPP

#include <map>
#include <string>
#include <mutex>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Types.hpp"
#include "Options.hpp"
#include "HttpClientBase.hpp"

namespace pt = boost::property_tree;
using namespace std;

class HttpClientExchangeInfo : public HttpClientBase {
    set<string>& active_symbols_;

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

        for(auto& srec: pt.get_child("symbols")) {
            const string symbol = srec.second.get<string>("symbol");

            if(srec.second.get<string>("status") == "TRADING") {
                active_symbols_.insert(symbol);
            }
        }

        { LOG(trace) << "HttpClientExchangeInfo: found " << active_symbols_.size() << " active symbols"; }

        success_ = true;
    }
public:
    void fetch() {
        get(options.val<string>("binance.api"), "/api/v3/exchangeInfo");        
    }

    HttpClientExchangeInfo(net::io_context &ioc, ssl::context &ctx, set<string>& active_symbols) 
    : HttpClientBase(ioc, ctx),
      active_symbols_(active_symbols)
    {
           
    }
        
};


#endif
