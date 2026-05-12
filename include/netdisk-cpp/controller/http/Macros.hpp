#pragma once

#define NETDISK_CONTROLLER_REQUEST(name)                                                           \
    auto name(boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,          \
              boost::asio::ssl::stream<boost::beast::tcp_stream>& stream,                          \
              boost::beast::flat_buffer& buffer, const boost::urls::matches& match)                \
        -> boost::asio::awaitable<Request>

#define NETDISK_CONTROLLER_RESPONSE(name)                                                          \
    auto name(Connection& connection, const boost::urls::matches& match, Config& config)           \
        -> boost::asio::awaitable<void>
