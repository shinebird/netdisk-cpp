#pragma once

#include "netdisk-cpp/core/http/Types.hpp"

#define NETDISK_CONTROLLER_REQUEST(name)                                                           \
    auto name(boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,          \
              core::http::SSLSocketType& stream,                          \
              boost::beast::flat_buffer& buffer, const boost::urls::matches& match,                \
              ::netdisk::core::http::Config& config, std::uint64_t connection_id,                  \
              std::any& extra_data) -> boost::cobalt::task<::netdisk::core::http::Request>

#define NETDISK_CONTROLLER_RESPONSE(name)                                                          \
    auto name(::netdisk::core::http::Connection& connection, const boost::urls::matches& match,    \
              ::netdisk::core::http::Config& config, std::uint64_t connection_id,                  \
              std::any& extra_data) -> boost::cobalt::task<void>
