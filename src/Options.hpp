#ifndef __options_hpp
#define __options_hpp

#include <string>

#include <boost/program_options.hpp>

#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>


using namespace std;
namespace po = boost::program_options;

#define LOG(severity) BOOST_LOG_SEV(options.logger, severity)

enum severity_level
{
    fatal,
    error,
    warning,
    info,
    debug,
    trace
};


class Options {
    private:
        po::variables_map vm;

        string make_usage_string(const string&, 
                                 const po::options_description&) const;

        void opp_add_general_options(po::options_description&) const;
        void opp_add_config_file(po::options_description& ) const;

        bool read_from_config_files(po::options_description&);

        void setup_logging_console();
        void setup_logging_file();

    public:
        boost::log::sources::severity_logger<severity_level> logger;

        Options() { }

        template<typename T> T val(const string& key) const {
            return (vm.count(key)?vm[key].as<T>():T()); }

        int count(const string& key) const { return vm.count(key);}

        bool parse(int, char**);

        string dump_variable_map() const;
};

extern Options options;

#endif