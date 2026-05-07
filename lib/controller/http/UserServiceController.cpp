#include "netdisk-cpp/controller/http/UserServiceController.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/http/read.hpp>

namespace netdisk::core::http::controller
{
    namespace request
    {
        namespace internal
        {
            template <typename Body, bool DiscardBody>
            static auto defaultHandler(
                boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,
                boost::asio::ssl::stream<boost::beast::tcp_stream>& stream,
                boost::beast::flat_buffer& buffer) -> boost::asio::awaitable<Request>
            {
                if constexpr (!DiscardBody)
                {
                    boost::beast::http::request_parser<Body> new_parser{std::move(parser)};
                    co_await boost::beast::http::async_read(stream, buffer, new_parser,
                                                            boost::asio::use_awaitable);
                    co_return pro::make_proxy<proxy::Request>(std::move(new_parser.get()));
                }
                else
                {
                    boost::beast::http::request_parser<boost::beast::http::dynamic_body> new_parser{
                        std::move(parser)};
                    while (!new_parser.is_done())
                    {
                        co_await boost::beast::http::async_read_some(stream, buffer, new_parser,
                                                                     boost::asio::use_awaitable);
                    }
                    co_return pro::make_proxy<proxy::Request>(std::move(new_parser.get()));
                }
            }
        } // namespace internal

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
