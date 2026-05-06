#include "netdisk-cpp/core/http/Config.hpp"

namespace netdisk::core::http
{
    Config::Config(std::uint16_t port, std::uint64_t num_threads)
        : port_(port), num_threads_(num_threads)
    {
    }

    auto Config::getCORS() -> config::CORS& { return this->CORS_; }

    auto Config::getPort() const -> std::uint16_t { return this->port_; }

    auto Config::getNumThreads() const -> std::uint64_t { return this->num_threads_; }
} // namespace netdisk::core::http
