#include "netdisk-cpp/controller/http/Common.hpp"
#include "netdisk-cpp/controller/http/UserServiceController.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/http/read.hpp>

namespace netdisk::core::http::controller
{
    namespace request
    {
        auto test(boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,
                  boost::asio::ssl::stream<boost::beast::tcp_stream>& stream,
                  boost::beast::flat_buffer& buffer, const boost::urls::matches& match)
            -> boost::asio::awaitable<Request>
        {
            co_return co_await internal::defaultHandler<boost::beast::http::dynamic_body, true>(
                parser, stream, buffer);
        }
    } // namespace request
    namespace response
    {
        auto test(Connection& connection, const boost::urls::matches& match, Config& config)
            -> boost::asio::awaitable<void>
        {
            co_await connection.stringReply("Hello world!", config);
        }
    } // namespace response
} // namespace netdisk::core::http::controller
