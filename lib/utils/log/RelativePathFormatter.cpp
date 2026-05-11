#include "netdisk-cpp/utils/log/RelativePathFormatter.hpp"

#include <filesystem>

namespace netdisk::utils::log
{
    void RelativePathFormatter::format(const spdlog::details::log_msg& msg, const std::tm&,
                                       spdlog::memory_buf_t& dest)
    {
        std::filesystem::path abs_path(msg.source.filename);
        std::filesystem::path base(NETDISK_BASE_DIR);

        std::error_code error_code;
        std::filesystem::path rel_path = std::filesystem::relative(abs_path, base, error_code);
        std::string output_str = error_code ? abs_path.filename().string() : rel_path.string();

        dest.append(output_str.data(), output_str.data() + output_str.size());
    }

    auto RelativePathFormatter::clone() const -> std::unique_ptr<spdlog::custom_flag_formatter>
    {
        return spdlog::details::make_unique<RelativePathFormatter>();
    }
} // namespace netdisk::utils::log
