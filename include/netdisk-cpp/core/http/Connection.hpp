#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/cobalt.hpp>


#include <proxy/v4/proxy.h>

#include <string_view>

#include "netdisk-cpp/core/http/Config.hpp"
#include "netdisk-cpp/core/http/Request.hpp"
#include "netdisk-cpp/core/http/Types.hpp"

namespace netdisk::core::http
{
    class Connection
    {
        public:
            Connection(SSLSocketType& socket) : socket_(socket) {}
            auto stringReply(std::string_view msg, Config& config) -> boost::cobalt::task<void>;

            auto fileReply(std::string_view path, Config& config) -> boost::cobalt::task<void>;

            auto errorReply(boost::beast::http::status status, std::string_view msg, Config& config)
                -> boost::cobalt::task<void>;

            auto staticBodyReply(boost::beast::http::status status, std::string_view msg,
                                 std::size_t msg_size, std::string_view mime_type, Config& config,
                                 const boost::beast::http::fields& extra_fields = {})
                -> boost::cobalt::task<void>;

            auto staticBodyReplyWithETag(boost::beast::http::status status, std::string_view msg,
                                         std::size_t msg_size, std::string_view mime_type,
                                         std::string_view e_tag, Config& config)
                -> boost::cobalt::task<void>;

            auto optionsReply(Config& config) -> boost::cobalt::task<void>;

            auto redirectReply(std::string_view new_target, Config& config)
                -> boost::cobalt::task<void>;

            template <typename Req> void setRequest(Req& req)
            {
                request_ = pro::make_proxy_view<proxy::Request>(req);
            }

            void setRequestProxy(Request req);
            auto getRequestProxy() -> Request&;

        private:
            boost::beast::error_code error_code_;
            SSLSocketType& socket_;
            Request request_;
    };
} // namespace netdisk::core::http
