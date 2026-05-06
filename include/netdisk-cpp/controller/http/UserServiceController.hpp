#pragma once

#include "netdisk-cpp/core/http/Config.hpp"
#include "netdisk-cpp/core/http/Connection.hpp"
#include "netdisk-cpp/core/http/Request.hpp"
#include "netdisk-cpp/utils/url/Matches.hpp"

namespace netdisk::core::http::controller
{
    namespace request
    {
        auto test(boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,
                  boost::asio::ssl::stream<boost::beast::tcp_stream>& stream,
                  boost::beast::flat_buffer& buffer, const boost::urls::matches& match)
            -> boost::asio::awaitable<Request>;
    }
    namespace response
    {
        auto test(Connection& connection, const boost::urls::matches& match, Config& config)
            -> boost::asio::awaitable<void>;
    }
} // namespace netdisk::core::http::controller
