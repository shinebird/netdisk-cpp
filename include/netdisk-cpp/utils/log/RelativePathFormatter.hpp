#pragma once

#include <spdlog/pattern_formatter.h>
#include <spdlog/spdlog.h>


namespace netdisk::utils::log
{
    class RelativePathFormatter : public spdlog::custom_flag_formatter
    {
        public:
            void format(const spdlog::details::log_msg& msg, const std::tm&,
                        spdlog::memory_buf_t& dest) override;
            [[nodiscard]] auto clone() const
                -> std::unique_ptr<spdlog::custom_flag_formatter> override;
    };
} // namespace netdisk::utils::log
