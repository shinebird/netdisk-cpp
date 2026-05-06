#pragma once

#include <proxy/proxy.h>

#include <boost/beast/http.hpp>

#include <cstdint>

PRO_DEF_MEM_DISPATCH(MemHttpResponseVersion, version);
PRO_DEF_MEM_DISPATCH(MemHttpResponseKeepAlive, keep_alive);
PRO_DEF_MEM_DISPATCH(MemHttpResponseSwap, swap);
PRO_DEF_MEM_DISPATCH(MemHttpResponseParserGet, get);

namespace netdisk::core::http
{
    namespace proxy
    {
        struct Response
            : public pro::facade_builder::add_convention<MemHttpResponseVersion,
                                                         std::uint32_t() const noexcept>::
                  add_convention<MemHttpResponseKeepAlive, bool() const>::add_convention<
                      MemHttpResponseSwap, void(boost::beast::http::fields&)>::
                      support_copy<pro::constraint_level::nontrivial>::build
        {
        };

        struct ResponseParser : public pro::facade_builder::add_convention<MemHttpResponseParserGet,
                                                                           Response&()>::build
        {
        };
    } // namespace proxy

    using ResponseView = pro::proxy_view<proxy::Response>;
    using Response = pro::proxy<proxy::Response>;
    using ResponseParserView = pro::proxy_view<proxy::ResponseParser>;
    using ResponseParser = pro::proxy<proxy::ResponseParser>;

} // namespace netdisk::core::http
