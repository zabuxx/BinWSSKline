#ifndef __HTTPCLIENTPRICE_HPP
#define __HTTPCLIENTPRICE_HPP

#include <list>
#include <map>
#include <string>
#include <algorithm>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Types.hpp"
#include "Options.hpp"
#include "HttpClientBase.hpp"

namespace pt = boost::property_tree;
using namespace std;


class HttpClientPrice : public HttpClientBase {
    const set<string>& active_symbols_;
    map<string, curfloat>& symbols_price_;


    void closure_action(bool fault_occured) {
        if(fault_occured) {
            LOG(fatal) << "HttpClientExchangeInfo: failed!";
            return;
        }

        stringstream ss(res_.body());
        pt::ptree pt;
        pt::read_json(ss, pt);

        for(auto& prec: pt) {
            const string symbol = prec.second.get<string>("symbol");
        
            if(find(active_symbols_.begin(), active_symbols_.end(), symbol) != active_symbols_.end())
                symbols_price_[symbol] = stold(prec.second.get<string>("price"));   
        }

        { LOG(trace) << "HttpClientPrice: loaded " << symbols_price_.size() << " prices"; }
    }

public:
    void fetch() {
        get(options.val<string>("binance.api"), "/api/v3/ticker/price");   
    }

    HttpClientPrice(net::io_context &ioc, ssl::context &ctx, const set<string>& active_symbols, map<string, curfloat>& symbols_price) 
    : HttpClientBase(ioc, ctx),
        active_symbols_(active_symbols),
        symbols_price_(symbols_price) {}    

};
#endif