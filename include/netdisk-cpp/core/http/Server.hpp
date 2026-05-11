#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/system/error_code.hpp>

#include <cstdint>

#include "netdisk-cpp/core/http/Config.hpp"
#include "netdisk-cpp/core/http/Connection.hpp"
#include "netdisk-cpp/core/http/Request.hpp"
#include "netdisk-cpp/utils/log/Logger.hpp"
#include "netdisk-cpp/utils/url/Matches.hpp"
#include "netdisk-cpp/utils/url/Router.hpp"

namespace netdisk::core::http
{
    using RequestHandler = std::function<boost::asio::awaitable<Request>(
        boost::beast::http::request_parser<boost::beast::http::empty_body>&,
        boost::asio::ssl::stream<boost::beast::tcp_stream>&, boost::beast::flat_buffer&,
        const boost::urls::matches&)>;
    using ResponseHandler = std::function<boost::asio::awaitable<void>(
        Connection&, const boost::urls::matches&, Config&)>;

    class Server
    {
        public:
            Server(std::uint16_t port, std::uint64_t num_threads);
            Server(const Server&) = delete;
            auto operator=(const Server&) -> Server& = delete;
            Server(Server&& other) noexcept = delete;
            [[nodiscard]] auto getConfig() -> Config&;
            auto initSSL() -> void;
            auto addRequestHandler(std::string_view pattern, RequestHandler&& handler) -> void;
            auto addResponseHandler(std::string_view pattern, ResponseHandler&& handler) -> void;
            auto addStaticFileRequestHandler(std::string_view pattern, RequestHandler&& handler) -> void;
            auto addStaticFileResponseHandler(std::string_view pattern, ResponseHandler&& handler) -> void;
            auto setLogger(std::shared_ptr<spdlog::logger> logger) -> void;
            auto run() -> void;

        private:
            Config config_;
            boost::urls::router<RequestHandler> request_router_;
            boost::urls::router<ResponseHandler> response_router_;
            boost::urls::router<RequestHandler> static_file_request_router_;
            boost::urls::router<ResponseHandler> static_file_response_router_;
            boost::asio::ssl::context ssl_context_;
            boost::asio::io_context io_context_;
            boost::system::error_code error_code_;
            std::shared_ptr<spdlog::logger> logger_;

            auto doListen(boost::asio::ip::tcp::endpoint endpoint) -> boost::asio::awaitable<void>;
            auto doSession(boost::asio::ssl::stream<boost::beast::tcp_stream> stream)
                -> boost::asio::awaitable<void>;
            auto handleRequest(
                boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,
                boost::asio::ssl::stream<boost::beast::tcp_stream>& stream,
                boost::beast::flat_buffer& buffer) -> boost::asio::awaitable<Request>;
            auto handleStaticFileRequest(
                boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,
                boost::asio::ssl::stream<boost::beast::tcp_stream>& stream,
                boost::beast::flat_buffer& buffer) -> boost::asio::awaitable<Request>;
            auto handleResponse(Connection connection) -> boost::asio::awaitable<void>;
            auto handleStaticFileResponse(Connection connection) -> boost::asio::awaitable<void>;
    };
} // namespace netdisk::core::http
