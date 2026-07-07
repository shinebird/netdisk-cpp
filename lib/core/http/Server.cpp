#include "netdisk-cpp/core/http/Server.hpp"
#include "netdisk-cpp/core/http/Connection.hpp"
#include "netdisk-cpp/core/http/Request.hpp"
#include "netdisk-cpp/utils/log/Logger.hpp"
#include "netdisk-cpp/utils/log/formatter/boost/stacktrace/stacktrace.hpp"
#include "netdisk-cpp/utils/ssl/SSL.hpp"
#include "netdisk-cpp/utils/url/Matches.hpp"

#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/impl/read.hpp>
#include <boost/cobalt.hpp>
#include <boost/stacktrace/stacktrace.hpp>
#include <boost/stacktrace/this_thread.hpp>
#include <boost/url.hpp>

#include <exception>
#include <proxy/v4/proxy.h>

#include <spdlog/spdlog.h>
#include <utility>
#include <vector>

namespace netdisk::core::http
{
    Server::Server(std::uint16_t port, std::uint64_t num_threads,
#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE

                   repository::database::sqlite::Connection* database_connection,
#endif
                   controller::security::UserAuthenticator* user_authenticator_,
                   controller::http::security::AuthorizationManager* authorization_manager)
        : config_(port, num_threads,
#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE

                  database_connection,
#endif
                  user_authenticator_, authorization_manager),
          ssl_context_(boost::asio::ssl::context::tlsv13), io_context_(num_threads)
    {
        for (auto& item : request_routers_)
        {
            item = std::make_unique<boost::urls::router<RequestHandler>>();
        }
        for (auto& item : response_routers_)
        {
            item = std::make_unique<boost::urls::router<ResponseHandler>>();
        }
    }

    auto Server::getConfig() -> Config& { return config_; }

    auto Server::initSSL() -> void { utils::ssl::configureSSLContext(ssl_context_); }

    auto Server::addRequestHandler(boost::beast::http::verb method, std::string_view pattern,
                                   RequestHandler&& handler) -> void
    {
        request_routers_[std::to_underlying(method)]->insert(pattern, std::move(handler));
    }

    auto Server::addResponseHandler(boost::beast::http::verb method, std::string_view pattern,
                                    ResponseHandler&& handler) -> void
    {
        response_routers_[std::to_underlying(method)]->insert(pattern, std::move(handler));
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

    auto Server::setAuthorizationHandler(AuthorizationHandler&& handler) -> void
    {
        authorization_handler_ = std::move(handler);
    }

    auto Server::setLogger(std::shared_ptr<spdlog::logger> logger) -> void
    {
        this->logger_ = std::move(logger);
    }

    auto Server::run() -> void
    {
        auto const address = boost::asio::ip::make_address("::");
        boost::cobalt::spawn(io_context_,
                             doListen(boost::asio::ip::tcp::endpoint{address, config_.getPort()}),
                             [&](std::exception_ptr e)
                             {
                                 // if (e)
                                 // {
                                 //     try
                                 //     {
                                 //         std::rethrow_exception(e);
                                 //     }
                                 //     catch (std::exception const& e)
                                 //     {
                                 //         SPDLOG_LOGGER_ERROR(
                                 //             logger_, "An error occoured in session:
                                 //             {}\nStacktrace: \n{}", e.what(),
                                 //             boost::stacktrace::stacktrace::from_current_exception());
                                 //     }
                                 // }
                             });

        // Run the I/O service on the requested number of threads
        boost::stacktrace::this_thread::set_capture_stacktraces_at_throw(true);
        std::vector<std::jthread> threads;
        threads.reserve(config_.getNumThreads() - 1);
        for (auto i = config_.getNumThreads() - 1; i > 0; --i)
        {
            threads.emplace_back(
                [&]
                {
                    boost::stacktrace::this_thread::set_capture_stacktraces_at_throw(true);
                    io_context_.run();
                });
        }
        io_context_.run();
    }

    auto Server::doListen(boost::asio::ip::tcp::endpoint endpoint) -> boost::cobalt::task<void>
    {
        auto executor = co_await boost::cobalt::this_coro::executor;
        auto acceptor = AcceptorType{executor, endpoint};

        for (;;)
        {
            auto ssl_stream =
                SSLSocketType(SocketType{co_await acceptor.async_accept()}, ssl_context_);
            try
            {
                co_await ssl_stream.async_handshake(boost::asio::ssl::stream_base::server);
            }
            catch (const boost::system::system_error& e)
            {
                SPDLOG_LOGGER_WARN(logger_, "An error occurred while SSL handshake: {}\n{}",
                                   e.what(),
                                   boost::stacktrace::stacktrace::from_current_exception());
            }
            try
            {
                boost::cobalt::spawn(executor, doSession(std::move(ssl_stream)),
                                     boost::asio::detached);
            }
            catch (std::exception const& e)
            {
                SPDLOG_LOGGER_ERROR(logger_, "An error occoured in session: {}\nStacktrace: \n{}",
                                    e.what(),
                                    boost::stacktrace::stacktrace::from_current_exception());
            }
        }
    }

    auto Server::doSession(SSLSocketType stream) -> boost::cobalt::task<void>
    {
        // This buffer is required to persist across reads
        boost::beast::flat_buffer buffer;

        for (;;)
        {
            // Set the timeout.
            // stream.next_layer().expires_after(std::chrono::seconds(30));
            auto connection_id = current_connection_id_.fetch_add(1);
            boost::beast::http::request_parser<boost::beast::http::empty_body> header_parser;
            co_await boost::beast::http::async_read_header(stream, buffer, header_parser);
            auto request_view = pro::make_proxy_view<proxy::Request>(header_parser.get());
            const auto target = request_view->target();
            SPDLOG_LOGGER_INFO(logger_, "Processing {}", target);
            std::any connection_extra_data;
            Connection connection(stream);
            bool has_uncaught_exception = false;
            try
            {
                if (authorization_handler_(request_view, config_))
                {
                    const auto keep_alive = header_parser.keep_alive();
                    auto request = co_await handleRequest(header_parser, stream, buffer,
                                                          connection_id, connection_extra_data);
                    connection.setRequestProxy(std::move(request));
                    co_await handleResponse(std::move(connection), connection_id,
                                            connection_extra_data);
                    if (!keep_alive)
                    {
                        // This means we should close the connection, usually because
                        // the response indicated the "Connection: close" semantic.
                        break;
                    }
                }
                else
                {
                    header_parser.get().keep_alive(false);
                    connection.setRequestProxy(
                        pro::make_proxy<proxy::Request>(std::move(header_parser.get())));
                    std::string_view msg = "401 Unauthorized";
                    boost::beast::http::fields extra_fields;
                    extra_fields.set(boost::beast::http::field::connection, "close");
                    co_await connection.staticBodyReply(boost::beast::http::status::unauthorized,
                                                        msg, msg.size(), "text/plain", config_,
                                                        extra_fields);
                    break;
                }
            }
            catch (const std::exception& e)
            {
                SPDLOG_LOGGER_ERROR(
                    logger_,
                    "An error occoured while processing request/response: {}\nStacktrace: \n{}",
                    e.what(), boost::stacktrace::stacktrace::from_current_exception());
                has_uncaught_exception = true;
            }
            catch (...)
            {
                SPDLOG_LOGGER_ERROR(
                    logger_,
                    "An error occoured while processing request/response\nStacktrace: \n{}",
                    boost::stacktrace::stacktrace::from_current_exception());
                has_uncaught_exception = true;
            }
            if (has_uncaught_exception)
            {
                // why this crash?
                std::string_view msg = "500 Internal Server Error";
                co_await connection.staticBodyReply(
                    boost::beast::http::status::internal_server_error, msg, msg.size(),
                    "text/plain", config_);
                break;
            }
        }

        // At this point the connection is closed gracefully
        // we ignore the error because the client might have
        // dropped the connection already.
    }

    auto Server::handleRequest(
        boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,
        SSLSocketType& stream, boost::beast::flat_buffer& buffer, std::uint64_t connection_id,
        std::any& extra_data) -> boost::cobalt::task<Request>
    {
        auto& request_headers = parser.get();
        const auto target = request_headers.target();
        const auto method = request_headers.method();
        boost::urls::matches url_match;
        boost::urls::url url = *boost::urls::parse_relative_ref(target);
        if (const auto* handler = request_routers_[std::to_underlying(method)]->find(
                url.remove_query().encoded_segments(), url_match))
        {
            co_return co_await (*handler)(parser, stream, buffer, url_match, config_, connection_id,
                                          extra_data);
        }
        // If there's no suitable handler, try finding suitable static-file handler
        co_return co_await handleStaticFileRequest(parser, stream, buffer, connection_id,
                                                   extra_data);
    }

    auto Server::handleStaticFileRequest(
        boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,
        SSLSocketType& stream, boost::beast::flat_buffer& buffer, std::uint64_t connection_id,
        std::any& extra_data) -> boost::cobalt::task<Request>
    {
        auto& request_headers = parser.get();
        const auto target = request_headers.target();
        boost::urls::matches url_match;
        boost::urls::url url = *boost::urls::parse_relative_ref(target);
        if (const auto* handler =
                static_file_request_router_.find(url.remove_query().encoded_segments(), url_match))
        {
            co_return co_await (*handler)(parser, stream, buffer, url_match, config_, connection_id,
                                          extra_data);
        }
        // Do nothing if there's no suitable handler
        co_return pro::make_proxy<proxy::Request>(std::move(parser.get()));
    }

    auto Server::handleResponse(Connection connection, std::uint64_t connection_id,
                                std::any& extra_data) -> boost::cobalt::task<void>
    {
        const auto target = connection.getRequestProxy()->target();
        const auto method = connection.getRequestProxy()->method();
        boost::urls::matches url_match;
        boost::urls::url url = *boost::urls::parse_relative_ref(target);
        if (method == boost::beast::http::verb::options)
        {
            co_return co_await connection.optionsReply(config_);
        }
        if (const auto* handler = response_routers_[std::to_underlying(method)]->find(
                url.remove_query().encoded_segments(), url_match))
        {
            co_return co_await (*handler)(connection, url_match, config_, connection_id,
                                          extra_data);
        }
        if (method == boost::beast::http::verb::get)
        {
            const auto* handler =
                static_file_response_router_.find(url.remove_query().encoded_segments(), url_match);
            if (handler != nullptr)
            {
                co_return co_await (*handler)(connection, url_match, config_, connection_id,
                                              extra_data);
            }
        }
        SPDLOG_LOGGER_INFO(logger_, "HTTP 404 (Not Found): [{}] {}",
                           boost::beast::http::to_string(method), target);
        co_return co_await connection.errorReply(boost::beast::http::status::not_found,
                                                 "No such page", config_);
    }
} // namespace netdisk::core::http
