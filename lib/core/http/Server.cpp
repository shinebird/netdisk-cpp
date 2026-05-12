#include "netdisk-cpp/core/http/Server.hpp"
#include "netdisk-cpp/core/http/Connection.hpp"
#include "netdisk-cpp/utils/log/Logger.hpp"
#include "netdisk-cpp/utils/log/formatter/boost/stacktrace/stacktrace.hpp"
#include "netdisk-cpp/utils/ssl/SSL.hpp"
#include "netdisk-cpp/utils/url/Matches.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/impl/read.hpp>
#include <boost/stacktrace/stacktrace.hpp>

#include <vector>

namespace netdisk::core::http
{
    Server::Server(std::uint16_t port, std::uint64_t num_threads)
        : config_(port, num_threads), ssl_context_(boost::asio::ssl::context::tlsv13),
          io_context_(num_threads)
    {
    }

    auto Server::getConfig() -> Config& { return config_; }

    auto Server::initSSL() -> void { utils::ssl::configureSSLContext(ssl_context_); }

    auto Server::addRequestHandler(std::string_view pattern, RequestHandler&& handler) -> void
    {
        request_router_.insert(pattern, std::move(handler));
    }

    auto Server::addResponseHandler(std::string_view pattern, ResponseHandler&& handler) -> void
    {
        response_router_.insert(pattern, std::move(handler));
    }

    auto Server::addStaticFileRequestHandler(std::string_view pattern, RequestHandler&& handler)
        -> void
    {
        static_file_request_router_.insert(pattern, std::move(handler));
    }

    auto Server::addStaticFileResponseHandler(std::string_view pattern, ResponseHandler&& handler)
        -> void
    {
        static_file_response_router_.insert(pattern, std::move(handler));
    }

    auto Server::setLogger(std::shared_ptr<spdlog::logger> logger) -> void
    {
        this->logger_ = std::move(logger);
    }

    auto Server::run() -> void
    {
        auto const address = boost::asio::ip::make_address("::");
        boost::asio::co_spawn(
            io_context_, doListen(boost::asio::ip::tcp::endpoint{address, config_.getPort()}),
            [&](std::exception_ptr e)
            {
                if (e)
                {
                    try
                    {
                        std::rethrow_exception(e);
                    }
                    catch (std::exception const& e)
                    {
                        logger_->error("An error occoured in session: {}\nStacktrace: \n{}",
                                       e.what(), boost::stacktrace::stacktrace());
                    }
                }
            });

        // Run the I/O service on the requested number of threads
        std::vector<std::jthread> threads;
        threads.reserve(config_.getNumThreads() - 1);
        for (auto i = config_.getNumThreads() - 1; i > 0; --i)
        {
            threads.emplace_back([&] { io_context_.run(); });
        }
        io_context_.run();
    }

    auto Server::doListen(boost::asio::ip::tcp::endpoint endpoint) -> boost::asio::awaitable<void>
    {
        auto executor = co_await boost::asio::this_coro::executor;
        auto acceptor = boost::asio::ip::tcp::acceptor{executor, endpoint};

        for (;;)
        {
            auto ssl_stream = boost::asio::ssl::stream<boost::beast::tcp_stream>(
                boost::beast::tcp_stream{
                    co_await acceptor.async_accept(boost::asio::use_awaitable)},
                ssl_context_);
            try
            {
                co_await ssl_stream.async_handshake(boost::asio::ssl::stream_base::server,
                                                    boost::asio::use_awaitable);
            }
            catch (const boost::system::system_error& e)
            {
                SPDLOG_LOGGER_WARN(logger_, "An error occurred while SSL handshake: {}\n{}",
                                   e.what(), boost::stacktrace::stacktrace());
            }
            try
            {
                boost::asio::co_spawn(executor, doSession(std::move(ssl_stream)),
                                      boost::asio::detached);
            }
            catch (std::exception const& e)
            {
                SPDLOG_LOGGER_ERROR(logger_, "An error occoured in session: {}\nStacktrace: \n{}",
                                    e.what(), boost::stacktrace::stacktrace());
            }
        }
    }

    auto Server::doSession(boost::asio::ssl::stream<boost::beast::tcp_stream> stream)
        -> boost::asio::awaitable<void>
    {
        // This buffer is required to persist across reads
        boost::beast::flat_buffer buffer;

        for (;;)
        {
            // Set the timeout.
            stream.next_layer().expires_after(std::chrono::seconds(30));

            // Read a request
            // boost::beast::http::request<boost::beast::http::string_body> request;
            // co_await boost::beast::http::async_read(stream, buffer, request);
            boost::beast::http::request_parser<boost::beast::http::empty_body> header_parser;
            co_await boost::beast::http::async_read_header(stream, buffer, header_parser,
                                                           boost::asio::use_awaitable);
            auto request = co_await handleRequest(header_parser, stream, buffer);
            Connection connection(stream);
            connection.setRequestProxy(std::move(request));
            co_await handleResponse(std::move(connection));
            // std::println("doSession loop finish");

            // Handle the request
            // boost::beast::http::message_generator msg = handle_request(*doc_root,
            // std::move(request));

            // Determine if we should close the connection
            // bool keep_alive = msg.keep_alive();

            // Send the response
            // co_await boost::beast::async_write(stream, std::move(msg));
        }

        // At this point the connection is closed gracefully
        // we ignore the error because the client might have
        // dropped the connection already.
    }

    auto Server::handleRequest(
        boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,
        boost::asio::ssl::stream<boost::beast::tcp_stream>& stream,
        boost::beast::flat_buffer& buffer) -> boost::asio::awaitable<Request>
    {
        auto& request_headers = parser.get();
        const auto target = request_headers.target();
        boost::urls::matches url_match;
        if (const auto* handler = request_router_.find(target, url_match))
        {
            co_return co_await (*handler)(parser, stream, buffer, url_match);
        }
        // If there's no suitable handler, try finding suitable static-file handler
        co_return co_await handleStaticFileRequest(parser, stream, buffer);
    }

    auto Server::handleStaticFileRequest(
        boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,
        boost::asio::ssl::stream<boost::beast::tcp_stream>& stream,
        boost::beast::flat_buffer& buffer) -> boost::asio::awaitable<Request>
    {
        auto& request_headers = parser.get();
        const auto target = request_headers.target();
        boost::urls::matches url_match;
        if (const auto* handler = static_file_request_router_.find(target, url_match))
        {
            co_return co_await (*handler)(parser, stream, buffer, url_match);
        }
        // Do nothing if there's no suitable handler
        co_return pro::make_proxy<proxy::Request>(std::move(parser.get()));
    }

    auto Server::handleResponse(Connection connection) -> boost::asio::awaitable<void>
    {
        const auto target = connection.getRequestProxy()->target();
        boost::urls::matches url_match;
        if (connection.getRequestProxy()->method() == boost::beast::http::verb::options)
        {
            co_return co_await connection.optionsReply(config_);
        }
        if (const auto* handler = response_router_.find(target, url_match))
        {
            co_return co_await (*handler)(connection, url_match, config_);
        }
        if (const auto* handler = static_file_response_router_.find(target, url_match))
        {
            co_return co_await (*handler)(connection, url_match, config_);
        }
        logger_->info("HTTP 404 (Not Found): {}", target);
        co_return co_await connection.errorReply(boost::beast::http::status::not_found,
                                                 "No such page", config_);
    }
} // namespace netdisk::core::http
