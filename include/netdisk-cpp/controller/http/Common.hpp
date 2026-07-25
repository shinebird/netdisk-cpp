#pragma once

#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/cobalt.hpp>
#include <boost/url.hpp>

#include "netdisk-cpp/core/http/Request.hpp"
#include "netdisk-cpp/core/http/Types.hpp"
#include "netdisk-cpp/utils/concept/Common.hpp"

namespace netdisk::controller::http
{
    inline auto getParam(const boost::urls::params_view& params,
                                const boost::core::string_view key) -> std::string
    {
        const auto iter = params.find(key);
        return {iter != params.end() ? (*iter).value : ""};
    }

    template <utils::reflect::StringLike... T> auto hasAllRequestParams(T&&... params) -> bool
    {
        return (!std::empty(std::forward<T>(params)) && ...);
    }

    namespace request
    {
        namespace internal
        {
            template <typename Body, bool DiscardBody = false>
            static auto defaultHandler(
                boost::beast::http::request_parser<boost::beast::http::empty_body>& parser,
                core::http::SSLSocketType& stream, boost::beast::flat_buffer& buffer)
                -> boost::cobalt::task<core::http::Request>
            {
                if constexpr (!DiscardBody)
                {
                    boost::beast::http::request_parser<Body> new_parser{std::move(parser)};
                    co_await boost::beast::http::async_read(stream, buffer, new_parser);
                    co_return pro::make_proxy<core::http::proxy::Request>(
                        std::move(new_parser.get()));
                }
                else
                {
                    boost::beast::http::request_parser<boost::beast::http::dynamic_body> new_parser{
                        std::move(parser)};
                    while (!new_parser.is_done())
                    {
                        co_await boost::beast::http::async_read_some(stream, buffer, new_parser);
                    }
                    co_return pro::make_proxy<core::http::proxy::Request>(
                        std::move(new_parser.get()));
                }
            }
        } // namespace internal
    } // namespace request

} // namespace netdisk::controller::http
