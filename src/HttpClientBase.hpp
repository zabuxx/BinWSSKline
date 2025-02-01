#ifndef _HttpClientBase_HPP
#define _HttpClientBase_HPP

//#include "ClientReport.hpp"
#include "StopWatch.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>

#include <iostream>


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

using namespace std;


/*
Generic async HTTP client base class

TODO:
 - use beast::error_code variable to print info on errors (as WSBase)

*/

class HttpClientBase : public enable_shared_from_this<HttpClientBase> 
{
protected:
    StopWatch<chrono::microseconds> stopwatch;

    tcp::resolver resolver_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_;  // must persist between reads
    http::request<http::empty_body> req_;
    http::response_parser<http::string_body> res_parser_;

    
    string hostname;

protected:
    bool success_;  

private:
    
    virtual void closure_action(bool fault_occured) = 0;
public:
    HttpClientBase(HttpClientBase&&) = default;

    HttpClientBase( net::io_context&, ssl::context&); 
    virtual ~HttpClientBase() { }


    void get(const string&, const string&); //starts the chain

    void on_resolve(beast::error_code, 
                    tcp::resolver::results_type);
    void on_connect(beast::error_code, 
                    tcp::resolver::results_type::endpoint_type);
    void on_handshake(beast::error_code);
    void on_write(beast::error_code, std::size_t);
    void on_read(beast::error_code, std::size_t);
    void on_shutdown(beast::error_code);

    bool success() const { return success_; }
};

#endif
