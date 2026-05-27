#include "netdisk-cpp/controller/http/Controller.hpp"
#include "netdisk-cpp/controller/http/StaticFileController.hpp"
#include "netdisk-cpp/controller/http/UserServiceController.hpp"
#include "netdisk-cpp/controller/http/security/LoginController.hpp"
#include "netdisk-cpp/core/http/Server.hpp"

namespace netdisk::controller::http
{
    void registerAll(::netdisk::core::http::Server& server)
    {
        server.addRequestHandler("/login", security::request::login);
        server.addResponseHandler("/login", security::response::login);

        server.addRequestHandler("/", request::mainPage);
        server.addResponseHandler("/", response::mainPage);
        server.addRequestHandler("/user/index", request::indexPage);
        server.addResponseHandler("/user/index", response::indexPage);
        server.addRequestHandler("/user/login", request::login);
        server.addResponseHandler("/user/login", response::login);
        server.addRequestHandler("/user/files", request::filePanel);
        server.addResponseHandler("/user/files", response::filePanel);
        server.addRequestHandler("/user/downloadAndExtract", request::downloadAndExtract);
        server.addResponseHandler("/user/downloadAndExtract", response::downloadAndExtract);
        server.addRequestHandler("/user/batchUploadFiles", request::batchUploadFiles);
        server.addResponseHandler("/user/batchUploadFiles", response::batchUploadFiles);

        server.addStaticFileRequestHandler("/user/{path+}", request::staticFile);
        server.addStaticFileResponseHandler("/user/{path+}", response::staticFile);
    }
} // namespace netdisk::controller::http
