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
            boost::beast::http::request_parser<boost::beast::http::dynamic_body> new_parser{
                std::move(parser)};
            co_await boost::beast::http::async_read(stream, buffer, new_parser, boost::asio::use_awaitable);
            co_return pro::make_proxy<proxy::Request>(std::move(new_parser.get()));
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
