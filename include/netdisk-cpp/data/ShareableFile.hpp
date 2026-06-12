#pragma once

#include <ctime>
#include <filesystem>

#include <boost/json/fwd.hpp>

namespace netdisk::data
{
    struct ShareableFile
    {
            ShareableFile() = default;
            explicit ShareableFile(const std::filesystem::path& path);
            std::size_t file_size_;
            std::filesystem::path file_name_;
            std::filesystem::path absolute_path_;
            bool is_directory_;
            std::time_t last_modified_;
    };

    // NOLINTBEGIN(readability-identifier-naming)
    void tag_invoke(const boost::json::value_from_tag&, boost::json::value& json_value,
                    const ShareableFile& shareable_file);
    // NOLINTEND(readability-identifier-naming)
} // namespace netdisk::data
