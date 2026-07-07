#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/cobalt.hpp>
#include <boost/system/error_code.hpp>

#include <any>
#include <array>
#include <atomic>
#include <cstdint>
#include <utility>

#include "netdisk-cpp/core/http/Config.hpp"
#include "netdisk-cpp/core/http/Connection.hpp"
#include "netdisk-cpp/core/http/Request.hpp"
#include "netdisk-cpp/core/http/Types.hpp"
#include "netdisk-cpp/utils/log/Logger.hpp"
#include "netdisk-cpp/utils/url/Matches.hpp"
#include "netdisk-cpp/utils/url/Router.hpp"

namespace netdisk::core::http
{
    class Server
    {
        public:
            Server(std::uint16_t port, std::uint64_t num_threads,
#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE
                   repository::database::sqlite::Connection* database_connection,
#endif
                   controller::security::UserAuthenticator* user_authenticator_,
                   controller::http::security::AuthorizationManager* authorization_manager);
            Server(const Server&) = delete;
            auto operator=(const Server&) -> Server& = delete;
            Server(Server&& other) noexcept = delete;
            [[nodiscard]] auto getConfig() -> Config&;
            auto initSSL() -> void;
            auto addRequestHandler(boost::beast::http::verb method, std::string_view pattern,
                                   RequestHandler&& handler) -> void;
            auto addResponseHandler(boost::beast::http::verb method, std::string_view pattern,
                                    ResponseHandler&& handler) -> void;
            auto addStaticFileRequestHandler(std::string_view pattern, RequestHandler&& handler)
                -> void;
            auto addStaticFileResponseHandler(std::string_view pattern, ResponseHandler&& handler)
                -> void;
            auto setAuthorizationHandler(AuthorizationHandler&& handler) -> void;
            auto setLogger(std::shared_ptr<spdlog::logger> logger) -> void;
            auto run() -> void;

        private:
            Config config_;
            std::array<std::unique_ptr<boost::urls::router<RequestHandler>>,
                       std::to_underlying(boost::beast::http::verb::unlink) + 1>
                request_routers_;
            std::array<std::unique_ptr<boost::urls::router<ResponseHandler>>,
                       std::to_underlying(boost::beast::http::verb::unlink) + 1>
                response_routers_;
            boost::urls::router<RequestHandler> static_file_request_router_;
            boost::urls::router<ResponseHandler> static_file_response_router_;
            AuthorizationHandler authorization_handler_;
            boost::asio::ssl::context ssl_context_;
            boost::asio::io_context io_context_;
            boost::system::error_code error_code_;
            std::shared_ptr<spdlog::logger> logger_;
            std::atomic_uint64_t current_connection_id_ = 0;

            auto doListen(boost::asio::ip::tcp::endpoint endpoint) -> boost::cobalt::task<void>;
            auto doSession(SSLSocketType stream) -> boost::cobalt::task<void>;
            auto handleRequest(
                boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,
                SSLSocketType& stream, boost::beast::flat_buffer& buffer,
                std::uint64_t connection_id, std::any& extra_data) -> boost::cobalt::task<Request>;
            auto handleStaticFileRequest(
                boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,
                SSLSocketType& stream, boost::beast::flat_buffer& buffer,
                std::uint64_t connection_id, std::any& extra_data) -> boost::cobalt::task<Request>;
            auto handleResponse(Connection connection, std::uint64_t connection_id,
                                std::any& extra_data) -> boost::cobalt::task<void>;
            auto handleStaticFileResponse(Connection connection, std::uint64_t connection_id,
                                          std::any& extra_data) -> boost::cobalt::task<void>;
    };
} // namespace netdisk::core::http
