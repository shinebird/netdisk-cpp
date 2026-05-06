#include "netdisk-cpp/core/http/Connection.hpp"
#include "netdisk-cpp/mime_types/MimeTypes.h"
#include "netdisk-cpp/utils/log/Logger.hpp"

#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/fields_fwd.hpp>
#include <boost/beast/version.hpp>

#include <filesystem>
#include <format>

namespace netdisk::core::http
{
#define ADD_CORS_HEADERS(request, res, error_code)                                                 \
    config.getCORS().addHeaders((request)->target(), pro::make_proxy_view<proxy::Response>(res),   \
                                error_code);                                                       \
    if (error_code)                                                                                \
    {                                                                                              \
        spdlog::get("multi_logger")                                                                \
            ->warn(R"(Unable to add CORS headers for response "{}")", (request)->target());        \
    }
#define COMMON_SHUTDOWN_SSL                                                                        \
    if (!request_->keep_alive())                                                                   \
    {                                                                                              \
        if (socket_.next_layer().socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send,    \
                                                   this->error_code_))                             \
        {                                                                                          \
            spdlog::get("multi_logger")                                                            \
                ->warn("An error occoured while shutting down SSL connection: {}",                 \
                       this->error_code_.message());                                               \
        }                                                                                          \
    }

    void Connection::setRequestProxy(Request req) { request_ = std::move(req); }

    auto Connection::getRequestProxy() -> Request& { return request_; }

    auto Connection::errorReply(boost::beast::http::status status, std::string_view msg,
                                Config& config) -> boost::asio::awaitable<void>
    {

        boost::beast::http::response<boost::beast::http::string_body> res{status,
                                                                          request_->version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(request_->keep_alive());
        res.body() = msg;
        std::error_code error_code;
        ADD_CORS_HEADERS(request_, res, error_code)
        res.prepare_payload();
        co_await boost::beast::http::async_write(socket_, res, boost::asio::use_awaitable);
        COMMON_SHUTDOWN_SSL
    }

    auto Connection::fileReply(std::string_view path, Config& config)
        -> boost::asio::awaitable<void>
    {
        boost::beast::http::file_body::value_type body;
        // std::string jpath = path_cat(doc_root, path);
        body.open(path.data(), boost::beast::file_mode::scan, error_code_);
        if (error_code_ == boost::beast::errc::no_such_file_or_directory)
        {
            co_await errorReply(boost::beast::http::status::not_found,
                                std::format("The resource was not found in {}", path), config);
            co_return;
        }
        auto const size = body.size();
        boost::beast::http::response<boost::beast::http::file_body> res{
            std::piecewise_construct, std::make_tuple(std::move(body)),
            std::make_tuple(boost::beast::http::status::ok, request_->version())};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        // TODO: 单个文件扩展名可能对应多个MimeType
        res.set(boost::beast::http::field::content_type,
                *(utils::mime_type::getMimeTypes(std::filesystem::path{path}.extension().string())
                      .begin()));
        res.content_length(size);
        res.keep_alive(request_->keep_alive());
        std::error_code error_code;
        ADD_CORS_HEADERS(request_, res, error_code)
        co_await boost::beast::http::async_write(socket_, res, boost::asio::use_awaitable);
        COMMON_SHUTDOWN_SSL
    }

    auto Connection::stringReply(std::string_view msg, Config& config)
        -> boost::asio::awaitable<void>
    {
        boost::beast::http::response<boost::beast::http::string_body> res{
            boost::beast::http::status::ok, request_->version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(request_->keep_alive());
        res.body() = msg;
        res.prepare_payload();
        std::error_code error_code;
        ADD_CORS_HEADERS(request_, res, error_code)
        co_await boost::beast::http::async_write(socket_, res, boost::asio::use_awaitable);
        // std::println("wrote");
        COMMON_SHUTDOWN_SSL
        // std::println("shutdown");
    }

    auto Connection::optionsReply(Config& config) -> boost::asio::awaitable<void>
    {
        boost::beast::http::response<boost::beast::http::string_body> res{
            boost::beast::http::status::no_content, request_->version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(request_->keep_alive());
        res.prepare_payload();
        std::error_code error_code;
        ADD_CORS_HEADERS(request_, res, error_code)
        co_await boost::beast::http::async_write(socket_, res, boost::asio::use_awaitable);
        COMMON_SHUTDOWN_SSL
    }
} // namespace netdisk::core::http
