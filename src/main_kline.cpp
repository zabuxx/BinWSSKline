#include <iostream>
#include <regex>

#include <boost/algorithm/string.hpp>

#include "HttpClientExchangeInfo.hpp"
#include "HttpClientPrice.hpp"
#include "WSKline.hpp"

int main(int argc, char* argv[]) {
    if(!options.parse(argc,argv)) {
        cerr << "ERROR parsing command line" << endl;
        return 1;
    }

    { LOG(info) << "main()"; }

    net::io_context ioc;
    ssl::context ctx(ssl::context::tlsv12_client);

    set<string> all_active_symbols;
    bool had_success;
    do {
        auto hcei = make_shared<HttpClientExchangeInfo>(ioc, ctx, all_active_symbols);
        hcei->fetch();
        ioc.run();
        ioc.restart();

        had_success = hcei->success();

        if(!had_success) {
            LOG(warning) << "retrying HttpClientExchange";
        }

    } while(!had_success);

    set<string> active_symbols;
    for(auto& pair: all_active_symbols) 
	if(regex_search(pair, regex("BTC|BNB|SOL|ETH|TRY|XRP"))) 
	    active_symbols.insert(boost::algorithm::to_lower_copy(pair));


    if(active_symbols.size() > 1024) {
	LOG(warning) << "More than 1024 filtered active symbols: " << active_symbols.size();
    } else {
	LOG(trace) << "Filtered ctive symbols: " << active_symbols.size();
    }
    
	
    map<string, curfloat> symbols_price;
    do {
        auto hcp = make_shared<HttpClientPrice>(ioc, ctx, active_symbols, symbols_price);
        hcp->fetch();
        ioc.run();
        ioc.restart();

        had_success = hcp->success();
        if(!had_success) {
            LOG(warning) << "retrying HttpClientPrice";
        }
    } while(!had_success);

    while(true) {
        auto wsk = make_shared<WSKline>(ioc, ctx, active_symbols, symbols_price);
        wsk->run();
        ioc.run();
        ioc.restart();
    }

            
    return 0;
}


