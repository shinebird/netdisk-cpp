#pragma once

#include "netdisk-cpp/controller/http/Macros.hpp"
#include "netdisk-cpp/core/http/Config.hpp"
#include "netdisk-cpp/core/http/Connection.hpp"
#include "netdisk-cpp/core/http/Request.hpp"
#include "netdisk-cpp/utils/url/Matches.hpp"

namespace netdisk::core::http::controller
{
    namespace request
    {
        NETDISK_CONTROLLER_REQUEST(test);
        NETDISK_CONTROLLER_REQUEST(indexPage);
        NETDISK_CONTROLLER_REQUEST(login);
        NETDISK_CONTROLLER_REQUEST(filePanel);
        NETDISK_CONTROLLER_REQUEST(downloadAndExtract);
        NETDISK_CONTROLLER_REQUEST(batchUploadFiles);
    } // namespace request
    namespace response
    {
        NETDISK_CONTROLLER_RESPONSE(test);
        NETDISK_CONTROLLER_RESPONSE(indexPage);
        NETDISK_CONTROLLER_RESPONSE(login);
        NETDISK_CONTROLLER_RESPONSE(filePanel);
        NETDISK_CONTROLLER_RESPONSE(downloadAndExtract);
        NETDISK_CONTROLLER_RESPONSE(batchUploadFiles);
    } // namespace response
} // namespace netdisk::core::http::controller
