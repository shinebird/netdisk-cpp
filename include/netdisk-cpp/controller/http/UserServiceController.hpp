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
        NETDISK_CONTROLLER_REQUEST(mainPage);
        NETDISK_CONTROLLER_REQUEST(indexPage);
        NETDISK_CONTROLLER_REQUEST(login);
        NETDISK_CONTROLLER_REQUEST(filePanel);
        NETDISK_CONTROLLER_REQUEST(downloadAndExtractPage);
        NETDISK_CONTROLLER_REQUEST(batchUploadFilesPage);
    } // namespace request
    namespace response
    {
        NETDISK_CONTROLLER_RESPONSE(mainPage);
        NETDISK_CONTROLLER_RESPONSE(indexPage);
        NETDISK_CONTROLLER_RESPONSE(login);
        NETDISK_CONTROLLER_RESPONSE(filePanel);
        NETDISK_CONTROLLER_RESPONSE(downloadAndExtractPage);
        NETDISK_CONTROLLER_RESPONSE(batchUploadFilesPage);
    } // namespace response
} // namespace netdisk::controller::http
