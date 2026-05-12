#pragma once

#include <proxy/proxy.h>

#include <boost/beast/http.hpp>

#include <cstdint>

PRO_DEF_MEM_DISPATCH(MemHttpRequestVersion, version);
PRO_DEF_MEM_DISPATCH(MemHttpRequestKeepAlive, keep_alive);
PRO_DEF_MEM_DISPATCH(MemHttpRequestTarget, target);
PRO_DEF_MEM_DISPATCH(MemHttpRequestMethod, method);
PRO_DEF_MEM_DISPATCH(MemHttpRequestSwap, swap);
PRO_DEF_MEM_DISPATCH(MemHttpRequestAt, at);
PRO_DEF_MEM_DISPATCH(MemHttpRequestParserGet, get);

namespace netdisk::core::http
{
    namespace proxy
    {
        struct Request
            : public pro::facade_builder::add_convention<MemHttpRequestVersion,
                                                         std::uint32_t() const noexcept>::
                  add_convention<MemHttpRequestKeepAlive, bool() const>::add_convention<
                      MemHttpRequestTarget, boost::core::string_view()>::
                      add_convention<MemHttpRequestMethod, boost::beast::http::verb()>::
                          add_convention<MemHttpRequestSwap, void(boost::beast::http::fields&)>::
                              add_convention<
                                  MemHttpRequestAt,
                                  const boost::core::string_view(boost::beast::http::field) const,
                                  const boost::core::string_view(boost::core::string_view)
                                      const>::support_copy<pro::constraint_level::nontrivial>::build
        {
        };

        struct RequestParser
            : public pro::facade_builder::add_convention<MemHttpRequestParserGet, Request&()>::build
        {
        };
    } // namespace proxy

    using RequestView = pro::proxy_view<proxy::Request>;
    using Request = pro::proxy<proxy::Request>;
    using RequestParserView = pro::proxy_view<proxy::RequestParser>;
    using RequestParser = pro::proxy<proxy::RequestParser>;

} // namespace netdisk::core::http
