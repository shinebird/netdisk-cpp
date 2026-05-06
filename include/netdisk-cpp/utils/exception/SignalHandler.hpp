#pragma once

#include <string>

namespace netdisk::utils::exception::signal
{
    auto toString(int signal) -> std::string;
    void terminateHandler();
    void abortHandler(int signal);
    void interruptHandler(int signal);
} // namespace netdisk::utils::exception::signal
