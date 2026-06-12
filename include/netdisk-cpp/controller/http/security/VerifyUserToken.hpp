#pragma once

#include "netdisk-cpp/core/http/Config.hpp"
#include "netdisk-cpp/core/http/Request.hpp"


namespace netdisk::controller::http::security
{
    auto verifyUserToken(core::http::RequestView& request, core::http::Config& config) -> bool;
}
