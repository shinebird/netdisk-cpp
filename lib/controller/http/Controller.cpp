#include "netdisk-cpp/controller/http/Controller.hpp"
#include "netdisk-cpp/controller/http/StaticFileController.hpp"
#include "netdisk-cpp/controller/http/UserServiceController.hpp"
#include "netdisk-cpp/controller/http/security/AuthorizationManager.hpp"
#include "netdisk-cpp/controller/http/security/AuthorizationRules.hpp"
#include "netdisk-cpp/controller/http/security/LoginController.hpp"
#include "netdisk-cpp/core/http/Server.hpp"

#include <boost/beast/http/verb.hpp>

namespace netdisk::controller::http
{
    void registerAll(::netdisk::core::http::Server& server)
    {
        using enum boost::beast::http::verb;
        server.addRequestHandler(post, "/login", security::request::login);
        server.addResponseHandler(post, "/login", security::response::login);

        server.addRequestHandler(get, "/", request::mainPage);
        server.addResponseHandler(get, "/", response::mainPage);
        server.addRequestHandler(get, "/user/index", request::indexPage);
        server.addResponseHandler(get, "/user/index", response::indexPage);
        server.addRequestHandler(get, "/user/login", request::login);
        server.addResponseHandler(get, "/user/login", response::login);
        server.addRequestHandler(get, "/user/files", request::filePanel);
        server.addResponseHandler(get, "/user/files", response::filePanel);
        server.addRequestHandler(get, "/user/downloadAndExtract", request::downloadAndExtract);
        server.addResponseHandler(get, "/user/downloadAndExtract", response::downloadAndExtract);
        server.addRequestHandler(get, "/user/batchUploadFiles", request::batchUploadFiles);
        server.addResponseHandler(get, "/user/batchUploadFiles", response::batchUploadFiles);

        server.addStaticFileRequestHandler("/user/{path+}", request::staticFile);
        server.addStaticFileResponseHandler("/user/{path+}", response::staticFile);

        auto* authorization_manager = server.getConfig().getAuthorizationManager();
        authorization_manager->addAuthorizationHandler(get, "/service/{path+}",
                                                       security::authorization_rules::serviceGet);
        authorization_manager->addAuthorizationHandler(post, "/service/{path+}",
                                                       security::authorization_rules::servicePost);
    }
} // namespace netdisk::controller::http
