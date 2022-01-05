#ifndef __WSBASE_HPP
#define __WSBASE_HPP


#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>


#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "StopWatch.hpp"


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using namespace std;

class WSBase : public enable_shared_from_this<WSBase> {
    StopWatch<chrono::microseconds> stopwatch;

    tcp::resolver resolver_;
    websocket::stream<
        beast::ssl_stream<beast::tcp_stream>> ws_;
    beast::flat_buffer buffer_;
    string host_;
    string port_;
    string text_;

protected:
    virtual void process_data(const string&) = 0;
public:
    explicit WSBase(net::io_context&, ssl::context&);
    virtual ~WSBase() { }

    void run(const string&, const string&,const string&);
    void on_resolve(beast::error_code, tcp::resolver::results_type);
    void on_connect(beast::error_code, tcp::resolver::results_type::endpoint_type);
    void on_ssl_handshake(beast::error_code);
    void on_handshake(beast::error_code);
    void on_read(beast::error_code, std::size_t);
};

#endif