#pragma once

#include "netdisk-cpp/core/http/config/CORS.hpp"

#include <cstdint>

namespace netdisk::core::http
{
    class Config
    {
        public:
            Config(std::uint16_t port, std::uint64_t num_threads);
            [[nodiscard]] auto getCORS() -> config::CORS&;
            [[nodiscard]] auto getPort() const -> std::uint16_t;
            [[nodiscard]] auto getNumThreads() const -> std::uint64_t;

        private:
            std::uint16_t port_;
            std::uint64_t num_threads_;
            config::CORS CORS_;
    };
} // namespace netdisk::core::http
