#ifndef WSKLINE_HPP_
#define WSKLINE_HPP_

#include <time.h>

#include <map>
#include <string>
#include <set>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>

#include <pqxx/pqxx>

#include "Types.hpp"
#include "AccExt.hpp"
#include "WSBase.hpp"

namespace acc = boost::accumulators;
using namespace std;


/* 
 * Opens WebSocket for !bookTicker 
 *
 * Keeps updates in accumulators
 *
 */

class WSKline : public WSBase {
    //TODO: accumulator is too heavy in current form, can be reduced to better suit db needed data  

    typedef acc::accumulator_set<curfloat, acc::stats<acc::tag::min, acc::tag::max, acc::tag::mean, acc::tag::first, acc::tag::last > > SymbAcc;
    //typedef acc::accumulator_set<curfloat, acc::stats<acc::tag::min, acc::tag::max, acc::tag::first, acc::tag::last > > SymbAcc;

    map<string, SymbAcc> klinemap_;
    const map<string, pair<string, string>>& active_symbols_;

    pqxx::connection csql_;
    time_t last_second_;

    unsigned counter_;

    
    void process_data(const string&);
    void time_cycle_kline();
public:
    WSKline(net::io_context&, ssl::context&,const map<string, pair<string, string>>&,const map<string,curfloat>&);

    void run();
};

#endif
