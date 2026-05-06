#include "netdisk-cpp/utils/log/Logger.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


namespace netdisk::utils::log
{
    void Logger::setLogPath(std::string_view log_path)
    {
        if (!log_path.empty())
        {
            this->log_path_ = log_path;
        }
    }

    void Logger::setFileLogLevel(spdlog::level::level_enum level) { this->file_log_level_ = level; }

    void Logger::setConsoleLogLevel(spdlog::level::level_enum level)
    {
        this->console_log_level_ = level;
    }

    auto Logger::getLogger() const -> std::shared_ptr<spdlog::logger> { return this->logger_; }

    void Logger::init()
    {
        spdlog::set_pattern("[%H:%M:%S] [%l] [%s (%#)] %v");
        spdlog::init_thread_pool(8192, 3);
        async_file_sink_ =
            spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", log_path_)
                ->sinks()[0];
        console_sink_ = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        async_file_sink_->set_level(file_log_level_);
        console_sink_->set_level(console_log_level_);
        logger_ = std::make_shared<spdlog::logger>(
            "multi_logger", spdlog::sinks_init_list{async_file_sink_, console_sink_});
        spdlog::register_logger(logger_);
    }
} // namespace netdisk::utils::log
