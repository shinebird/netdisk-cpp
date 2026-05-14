#pragma once

#include "netdisk-cpp/controller/http/Macros.hpp"
#include "netdisk-cpp/core/http/Config.hpp"
#include "netdisk-cpp/core/http/Connection.hpp"
#include "netdisk-cpp/core/http/Request.hpp"
#include "netdisk-cpp/utils/url/Matches.hpp"

namespace netdisk::controller::http
{
    namespace request
    {
        NETDISK_CONTROLLER_REQUEST(staticFile);
    }

    namespace response
    {
        NETDISK_CONTROLLER_RESPONSE(staticFile);
    }
} // namespace netdisk::controller::http
