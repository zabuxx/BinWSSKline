#ifndef __STOPWATCH_HPP
#define __STOPWATCH_HPP

#include <iostream>
#include <vector>
#include <chrono>
#include <sstream>

using namespace std;

/*
    Allows to measure the time between events and store 
    subsequent results in vector

    By default in Microseconds
*/

template<class DurationType = chrono::microseconds>
class StopWatch
{
    using clock_type = chrono::high_resolution_clock;

    clock_type::time_point when_;

    vector<unsigned> stops;
    vector<string> msgs;

public:
    using duration = clock_type::duration;

    StopWatch()
        : when_(clock_type::now())
    {
    }

    void reset() { when_ = clock_type::now(); msgs.clear(); stops.clear();}

    void button_click(const string& msg = "", bool store_results = true) {
        if(store_results) {
            unsigned tdiff = chrono::duration_cast<DurationType>(clock_type::now() - when_).count();
            stops.push_back(tdiff);
            msgs.push_back(msg);
        }

        when_ = clock_type::now();
    }

    void add_penalty(unsigned p) { stops.push_back(p); }
    
    const vector<unsigned>& result() const { return stops; }

    const string result_msg() const { stringstream ss; for(unsigned i=0;i<stops.size();i++) ss << msgs[i] << ":" << stops[i] << "|"; return "<"+ss.str()+">";}

    duration
    elapsed() const
    {
        return clock_type::now() - when_;
    }
};


#endif