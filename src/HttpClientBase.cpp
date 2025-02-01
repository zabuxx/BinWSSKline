#include "HttpClientBase.hpp"
#include "Options.hpp"

#include <string>
#include <boost/algorithm/string.hpp>

HttpClientBase::HttpClientBase(net::io_context& ioc, ssl::context& ctx) :
    resolver_(net::make_strand(ioc)), // resolver_(net::make_strand(ioc)),  -> no strand needed as we will pass seperate io_context
    stream_(net::make_strand(ioc), ctx), //stream_(net::make_strand(ioc))       to avoid bottleneck on resolver
    success_(false)
{
    req_.version(11);
    req_.method(http::verb::get);
    
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    res_parser_.body_limit(std::numeric_limits<uint64_t>::max());
}

void HttpClientBase::get(const string& host, const string& uri) {
    hostname = host;
    req_.set(http::field::host, host);
    req_.target(uri);
    
    { LOG(trace) << "HttpClientBase: launch " << host << " / " << uri; }
    stopwatch.button_click("C++Init");

    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(2));

    resolver_.async_resolve(
        host,
        "https",
        beast::bind_front_handler(
         &HttpClientBase::on_resolve,
         shared_from_this()));
}

void HttpClientBase::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
{
    stopwatch.button_click("DNS");

    if(ec)  {
        LOG(warning) << "HttpClientBase: failed to resolve:" << hostname << ":https, MSG:"
                   << ec.message();
        closure_action(true);
        return;
    } /* else { 
        LOG(trace) << "HttpClientBase: resolved " << hostname << ":https"; 
    } */

    // Set maximum timeout for complete transaction
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(5));

    beast::get_lowest_layer(stream_).async_connect(
        results,
        beast::bind_front_handler(
            &HttpClientBase::on_connect,
            shared_from_this()));
}

void HttpClientBase::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
{
    stopwatch.button_click("TCP");

    if(ec)  {
        LOG(warning) << "HttpClientBase: failed to connect to:" << hostname << ":https, MSG: " 
                   << ec.message();
        closure_action(true);
        return;
    } /* else {
        LOG(trace) << "HttpClientBase: connected to: " << hostname << ":https";
    } */

    const string connection_host = hostname + ":443";
    if(! SSL_set_tlsext_host_name(
        stream_.native_handle(),
        connection_host.c_str())) {
        LOG(error) << "Failed to set SNI Hostname";
    }

    // Perform the SSL handshake
    stream_.async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(
            &HttpClientBase::on_handshake,
            shared_from_this()));
}

void HttpClientBase::on_handshake(beast::error_code ec)
{
    stopwatch.button_click("SSL");

    if(ec)  {
        LOG(warning) << "HttpClientBase: failed to SSL connect to:" << hostname << ":https, MSG: "
                   << ec.message();
        closure_action(true);
        return;
    } /* else {
        LOG(trace) << "HttpClientBase: SSL connected to: " << hostname << ":https";
    } */

    // Send the HTTP request to the remote host
    http::async_write(stream_, req_,
        beast::bind_front_handler(
            &HttpClientBase::on_write,
            shared_from_this()));
}


void HttpClientBase::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    stopwatch.button_click("SEND");

    if(ec) {
        LOG(warning) << "HttpClientBase: failed to write to:" << hostname << ":https, MSG:"
                   << ec.message();
        closure_action(true);
        return;
    } /* else {
        LOG(trace) << "HttpClientBase: request send to " << hostname << ":https";
    } */

    // Receive the HTTP response
    http::async_read(stream_, buffer_, res_parser_,
        beast::bind_front_handler(
            &HttpClientBase::on_read,
            shared_from_this()));
}

void HttpClientBase::on_read(beast::error_code ec,std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    stopwatch.button_click("RCV");

    if(ec) {
        LOG(warning) << "HttpClientBase: failed to read from:" << hostname << ":https, MSG:"
                   << ec.message();
        
        closure_action(true);
        return;
    } /* else {
        LOG(trace) << "HttpClientBase: response recieved from " << hostname << ":https";
        
    } */

    auto& res = res_parser_.get();
    
    if(res.result_int() != 200) {
        LOG(warning) << "HttpClientBase: ret code = " << res.result_int();
    }
    
    //check binance x-mbx headers:
    
    stringstream log_msg;
    for(auto const& field: res) {
        const string field_name = boost::to_upper_copy(string(field.name_string()));
        
        if(field_name.rfind("X-MBX", 0) == 0) {
            log_msg << "(" << field_name << ":" << field.value() << ")";
        }
    }
    { LOG(trace) << "HttpClientBase: " << log_msg.str(); }
    


    // Set a timeout on the operation
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(2));

    // Gracefully close the stream
    stream_.async_shutdown(
        beast::bind_front_handler(
            &HttpClientBase::on_shutdown,
            shared_from_this()));
}

void HttpClientBase::on_shutdown(beast::error_code ec)
{
    if(ec == net::error::eof)
    {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec = {};
    }
 
    stopwatch.button_click("PARSED");    

    //if(ec)
        //no need to report error on shutdown

    if(options.val<bool>("perf_detail")) {
        //string stopwatch_result = "<";
        //for(const auto v: stopwatch.result()) 
        //stopwatch_result += to_string(v) + "|";
        //stopwatch_result += ">";


        { LOG(trace) << "HttpClientBase: launching closure action for:" << hostname << stopwatch.result_msg(); }
    }

    
    closure_action(false);
    success_ = true;

    // If we get here then the connection is closed gracefully
}
