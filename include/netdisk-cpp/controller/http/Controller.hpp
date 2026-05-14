#pragma once

namespace netdisk::core::http
{
    class Server;
}

namespace netdisk::controller::http
{
    void registerAll(::netdisk::core::http::Server& server);
}
