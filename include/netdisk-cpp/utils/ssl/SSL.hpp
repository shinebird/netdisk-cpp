#pragma once

#include <boost/asio/ssl/context.hpp>

namespace netdisk::utils::ssl
{
    void configureSSLContext(boost::asio::ssl::context& ctx);
}
