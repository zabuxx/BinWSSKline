#include <unistd.h>
#include <cstdlib>

#include "WSBase.hpp"
#include "Options.hpp"

 WSBase::WSBase(net::io_context& ioc, ssl::context& ctx)
      : resolver_(net::make_strand(ioc)),
        ws_(net::make_strand(ioc), ctx)
{

}

void WSBase::run(const string& host, const string& port, const string& text)
{
    host_ = host;
    port_ = port;
    text_ = text;

    // Look up the domain name
    resolver_.async_resolve(
        host_,
        port_,
        beast::bind_front_handler(
            &WSBase::on_resolve,
            shared_from_this()));
}

void WSBase::on_resolve(
    beast::error_code ec,
    tcp::resolver::results_type results)
{
    stopwatch.button_click("DNS");

    if(ec) {
        //resolv failure is not an option, try again
        LOG(error) << "WSBase: failed to resolve (" << host_ << ":" << port_ << "):"
                    << ec.message();
        return;
    }

    // Set a timeout on the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(5));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(ws_).async_connect(
        results,
        beast::bind_front_handler(
            &WSBase::on_connect,
            shared_from_this()));
}


void WSBase::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
{
     stopwatch.button_click("TCP");

    if(ec) {
        //connect failure is not an option, try again
        LOG(error) << "WSBase: failed to TCP connect to (" << host_ << ":" << port_ << "):"
                   << ec.message();;
        return;
    }

    // Update the host_ string. This will provide the value of the
    // Host HTTP header during the WebSocket handshake.
    // See https://tools.ietf.org/html/rfc7230#section-5.4
    //host_ += ':' + port_;

    // Set a timeout on the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(3));

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if(! SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
        LOG(fatal) << "WSBase: Internal error setting SSL SNI";
        exit(11); //brutal, i know
    }

    // Perform the SSL handshake
    ws_.next_layer().async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(
            &WSBase::on_ssl_handshake,
            shared_from_this()));
}

void WSBase::on_ssl_handshake(beast::error_code ec)
{
    stopwatch.button_click("SSL");

    if(ec) {
        LOG(error) << "WSBase: failed to SSL connect to (" << host_ << ":" << port_  << "):"
                   << ec.message();
        return;
    }

    beast::get_lowest_layer(ws_).expires_never();

    //ws_.set_option(
    //    websocket::stream_base::timeout::suggested(
    //        beast::role_type::client));

    websocket::stream_base::timeout opt {
        std::chrono::seconds(5),   // handshake timeout
        std::chrono::seconds(10),  // idle timeout
        true                       // send keepalive pings at half idle interval
    };
    ws_.set_option(opt);

    // Set a decorator to change the User-Agent of the handshake
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::request_type& req)
        {
            req.set(http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " buzzy-socsuc v0.69");
        }));

    { LOG(trace) << "WSBase: connecting to: " << host_ << " " << text_; }

    // Perform the websocket handshake
    ws_.async_handshake(host_, text_,
        beast::bind_front_handler(
            &WSBase::on_handshake,
            shared_from_this()));
}

void WSBase::on_handshake(beast::error_code ec) {

    stopwatch.button_click("HNDSHK");

    if(ec) {
        LOG(error) << "WSBase: failed handshake with (" << host_ << ":" << port_ << "): "
                   << ec.message()
                   << (options.val<bool>("perf_detail")?stopwatch.result_msg():string(""));
        return;
    } else {
        LOG(trace) << "WSBase: reading from (" << host_ << ":" << port_ << ")"
                   << (options.val<bool>("perf_detail")?stopwatch.result_msg():string(""));
    }

    // Read a message into our buffer
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &WSBase::on_read,
            shared_from_this()));
}


void WSBase::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec) {
        LOG(error) << "WSBase: failed read from (" << host_ << ":" << port_ << "):"
                   << ec.message();
        return;
    }

    process_data(beast::buffers_to_string(buffer_.data()));
    buffer_.consume(buffer_.size());

    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &WSBase::on_read,
            shared_from_this()));
}
