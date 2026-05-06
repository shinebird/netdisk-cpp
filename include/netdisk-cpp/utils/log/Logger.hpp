#pragma once

#include <spdlog/sinks/sink.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <string_view>

namespace netdisk::utils::log
{
    class Logger
    {
        public:
            void setLogPath(std::string_view log_path);
            void setFileLogLevel(spdlog::level::level_enum level);
            void setConsoleLogLevel(spdlog::level::level_enum level);
            void init();
            [[nodiscard]] auto getLogger() const -> std::shared_ptr<spdlog::logger>;

        private:
            std::shared_ptr<spdlog::logger> logger_;
            std::shared_ptr<spdlog::sinks::sink> async_file_sink_;
            std::shared_ptr<spdlog::sinks::sink> console_sink_;
            std::string log_path_ = "log.txt";
            spdlog::level::level_enum file_log_level_;
            spdlog::level::level_enum console_log_level_;
    };
} // namespace netdisk::utils::log
