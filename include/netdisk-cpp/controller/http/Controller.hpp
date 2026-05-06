#pragma once

namespace netdisk::core::http
{
    class Server;
    namespace controller
    {
        void registerAll(Server& server);
    }
} // namespace netdisk::core::http
