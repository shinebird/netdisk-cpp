#include "netdisk-cpp/controller/http/Controller.hpp"
#include "netdisk-cpp/controller/http/UserServiceController.hpp"
#include "netdisk-cpp/core/http/Server.hpp"

namespace netdisk::core::http::controller
{
    void registerAll(Server& server)
    {
        server.addRequestHandler("/test", request::test);
        server.addResponseHandler("/test", response::test);
    }
} // namespace netdisk::core::http::controller
