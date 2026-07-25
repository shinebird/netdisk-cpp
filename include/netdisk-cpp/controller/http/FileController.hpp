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
        NETDISK_CONTROLLER_REQUEST(getShareableFiles);
        NETDISK_CONTROLLER_REQUEST(checkFileExists);
        NETDISK_CONTROLLER_REQUEST(batchDownloadFile);
    }

    namespace response
    {
        NETDISK_CONTROLLER_RESPONSE(getShareableFiles);
        NETDISK_CONTROLLER_RESPONSE(checkFileExists);
        NETDISK_CONTROLLER_RESPONSE(batchDownloadFile);
    }
} // namespace netdisk::controller::http
