#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <proxy/v4/proxy.h>

#include <string_view>

#include "netdisk-cpp/core/http/Config.hpp"
#include "netdisk-cpp/core/http/Request.hpp"

namespace netdisk::core::http
{
    class Connection
    {
        public:
            Connection(boost::asio::ssl::stream<boost::beast::tcp_stream>& socket) : socket_(socket)
            {
            }
            auto stringReply(std::string_view msg, Config& config) -> boost::asio::awaitable<void>;

            auto fileReply(std::string_view path, Config& config) -> boost::asio::awaitable<void>;

            auto errorReply(boost::beast::http::status status, std::string_view msg, Config& config)
                -> boost::asio::awaitable<void>;

            auto staticBodyReply(boost::beast::http::status status, std::string_view msg,
                                 std::size_t msg_size, std::string_view mime_type, Config& config,
                                 const boost::beast::http::fields& extra_fields = {})
                -> boost::asio::awaitable<void>;

            auto staticBodyReplyWithETag(boost::beast::http::status status, std::string_view msg,
                                         std::size_t msg_size, std::string_view mime_type,
                                         std::string_view e_tag, Config& config)
                -> boost::asio::awaitable<void>;

            auto optionsReply(Config& config) -> boost::asio::awaitable<void>;

            auto redirectReply(std::string_view new_target, Config& config)
                -> boost::asio::awaitable<void>;

            template <typename Req> void setRequest(Req& req)
            {
                request_ = pro::make_proxy_view<Request>(req);
            }

            void setRequestProxy(Request req);
            auto getRequestProxy() -> Request&;

        private:
            boost::beast::error_code error_code_;
            boost::asio::ssl::stream<boost::beast::tcp_stream>& socket_;
            Request request_;
    };
} // namespace netdisk::core::http
