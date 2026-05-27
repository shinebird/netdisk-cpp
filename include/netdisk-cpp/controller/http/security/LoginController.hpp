#pragma once

#include "netdisk-cpp/controller/http/Macros.hpp"
#include "netdisk-cpp/core/http/Config.hpp"
#include "netdisk-cpp/core/http/Connection.hpp"
#include "netdisk-cpp/core/http/Request.hpp"
#include "netdisk-cpp/utils/url/Matches.hpp"

namespace netdisk::controller::http::security
{
    namespace request
    {
        NETDISK_CONTROLLER_REQUEST(login);
    }

    namespace response
    {
        NETDISK_CONTROLLER_RESPONSE(login);
    }
} // namespace netdisk::controller::http::security
