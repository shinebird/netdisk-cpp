#include "netdisk-cpp/core/http/Connection.hpp"
#include "netdisk-cpp/mime_types/MimeTypes.hpp"
#include "netdisk-cpp/utils/log/Logger.hpp"

#include <boost/asio/redirect_error.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields_fwd.hpp>
#include <boost/beast/http/impl/write.hpp>
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
        SPDLOG_LOGGER_WARN(spdlog::get("multi_logger"),                                            \
                           R"(Unable to add CORS headers for response "{}")",                      \
                           (request)->target());                                                   \
    }
#define COMMON_SHUTDOWN_SSL                                                                        \
    if (!request_->keep_alive())                                                                   \
    {                                                                                              \
        if (socket_.next_layer().socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send,    \
                                                   this->error_code_))                             \
        {                                                                                          \
            SPDLOG_LOGGER_WARN(spdlog::get("multi_logger"),                                        \
                               "An error occoured while shutting down SSL connection: {}",         \
                               this->error_code_.message());                                       \
        }                                                                                          \
    }

    void Connection::setRequestProxy(Request req) { request_ = std::move(req); }

    auto Connection::getRequestProxy() -> Request& { return request_; }

    auto Connection::staticBodyReply(boost::beast::http::status status, std::string_view msg,
                                     std::size_t msg_size, std::string_view mime_type,
                                     Config& config, const boost::beast::http::fields& extra_fields)
        -> boost::asio::awaitable<void>
    {
        boost::beast::http::response<boost::beast::http::buffer_body> res{status,
                                                                          request_->version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, mime_type);
        res.keep_alive(request_->keep_alive());
        for (const auto& item : extra_fields)
        {
            res.insert(item.name(), item.value());
        }
        std::error_code error_code;
        ADD_CORS_HEADERS(request_, res, error_code)
        res.content_length(msg_size);
        // res.prepare_payload();
        boost::beast::http::response_serializer<boost::beast::http::buffer_body> serializer(res);
        co_await boost::beast::http::async_write_header(socket_, serializer,
                                                        boost::asio::use_awaitable);
        std::size_t offset = 0;
        constexpr std::size_t chunk_size = 64 * 1024;
        boost::beast::error_code ec;
        while (offset < msg_size)
        {
            std::size_t current_chunk_size = std::min(chunk_size, msg_size - offset);
            res.body().data = (void*)(msg.data() + offset);
            res.body().size = current_chunk_size;
            res.body().more = (offset + current_chunk_size < msg_size);
            co_await boost::beast::http::async_write(socket_, serializer,
                                                     boost::asio::redirect_error(ec));
            if (ec == boost::beast::http::error::need_buffer || (!ec))
            {
                offset += current_chunk_size;
                ec = {};
            }
            else
            {
                throw boost::beast::system_error(ec);
            }
        }
        res.body().data = nullptr;
        res.body().size = 0;
        res.body().more = false;
        co_await boost::beast::http::async_write(socket_, serializer, boost::asio::use_awaitable);
        COMMON_SHUTDOWN_SSL
    }

    auto Connection::staticBodyReplyWithETag(boost::beast::http::status status,
                                             std::string_view msg, std::size_t msg_size,
                                             std::string_view mime_type, std::string_view e_tag,
                                             Config& config) -> boost::asio::awaitable<void>
    {
        boost::beast::http::fields extra_fields;
        extra_fields.set(boost::beast::http::field::etag, e_tag);
        const auto if_none_match = request_->operator[](boost::beast::http::field::if_none_match);
        if (e_tag == if_none_match)
        {
            co_return co_await staticBodyReply(boost::beast::http::status::not_modified, "", 0,
                                               "text/html", config, extra_fields);
        }
        co_return co_await staticBodyReply(status, msg, msg_size, mime_type, config, extra_fields);
    }

    auto Connection::errorReply(boost::beast::http::status status, std::string_view msg,
                                Config& config) -> boost::asio::awaitable<void>
    {
        co_return co_await staticBodyReply(status, msg, msg.size(), "text/html", config);
    }

    auto Connection::fileReply(std::string_view path, Config& config)
        -> boost::asio::awaitable<void>
    {
        boost::beast::http::file_body::value_type body;
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
        co_return co_await staticBodyReply(boost::beast::http::status::ok, msg, msg.size(),
                                           "text/html", config);
    }

    auto Connection::optionsReply(Config& config) -> boost::asio::awaitable<void>
    {
        co_return co_await staticBodyReply(boost::beast::http::status::no_content, "", 0,
                                           "text/html", config);
    }

    auto Connection::redirectReply(std::string_view new_target, Config& config)
        -> boost::asio::awaitable<void>
    {
        boost::beast::http::response<boost::beast::http::string_body> res{
            boost::beast::http::status::moved_permanently, request_->version()};
        const auto host = request_->at(boost::beast::http::field::host);
        res.set(boost::beast::http::field::location, std::format("https://{}{}", host, new_target));
        res.body() = "";
        res.prepare_payload();
        res.keep_alive(request_->keep_alive());
        std::error_code error_code;
        ADD_CORS_HEADERS(request_, res, error_code)
        co_await boost::beast::http::async_write(socket_, res, boost::asio::use_awaitable);
        COMMON_SHUTDOWN_SSL
    }
} // namespace netdisk::core::http
