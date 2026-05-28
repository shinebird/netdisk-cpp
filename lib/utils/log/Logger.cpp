#include "netdisk-cpp/utils/log/Logger.hpp"
#include "netdisk-cpp/utils/log/RelativePathFormatter.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

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
        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->add_flag<RelativePathFormatter>('r').set_pattern("[%H:%M:%S] [%l] [%r(%#)] %v");
        spdlog::init_thread_pool(8192, 3);
        async_file_sink_ = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path_, true);
        console_sink_ = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        async_file_sink_->set_level(file_log_level_);
        console_sink_->set_level(console_log_level_);
        logger_ = std::make_shared<spdlog::async_logger>(
            "multi_logger", spdlog::sinks_init_list{async_file_sink_, console_sink_},
            spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        logger_->set_formatter(std::move(formatter));
        logger_->set_level(spdlog::level::trace);
        logger_->flush_on(spdlog::level::warn);
        spdlog::register_logger(logger_);
    }
} // namespace netdisk::utils::log
