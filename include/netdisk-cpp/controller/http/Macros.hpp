#pragma once

#define NETDISK_CONTROLLER_REQUEST(name)                                                           \
    auto name(boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,          \
              boost::asio::ssl::stream<boost::beast::tcp_stream>& stream,                          \
              boost::beast::flat_buffer& buffer, const boost::urls::matches& match,                \
              ::netdisk::core::http::Config& config, std::uint64_t connection_id,                  \
              std::any& extra_data) -> boost::asio::awaitable<::netdisk::core::http::Request>

#define NETDISK_CONTROLLER_RESPONSE(name)                                                          \
    auto name(::netdisk::core::http::Connection& connection, const boost::urls::matches& match,    \
              ::netdisk::core::http::Config& config, std::uint64_t connection_id,                  \
              std::any& extra_data) -> boost::asio::awaitable<void>
