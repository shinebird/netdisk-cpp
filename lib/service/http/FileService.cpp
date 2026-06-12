#include "netdisk-cpp/service/http/FileService.hpp"
#include "netdisk-cpp/data/ShareableFile.hpp"
#include "netdisk-cpp/utils/filesystem/ListRoots.hpp"

#include <algorithm>
#include <execution>
#include <filesystem>

namespace netdisk::service::http
{
    auto getShareableFiles(const std::expected<std::filesystem::path, std::string>& path)
        -> std::vector<data::ShareableFile>
    {
        std::vector<data::ShareableFile> result;
        if (path)
        {
            const auto file_status = std::filesystem::status(path.value());
            if (std::filesystem::exists(file_status))
            {
                if (file_status.type() == std::filesystem::file_type::regular)
                {
                    result.emplace_back(path.value());
                }
                if (file_status.type() == std::filesystem::file_type::directory)
                {
                    std::vector<std::filesystem::path> dir_paths;
                    for (const auto& entry : std::filesystem::directory_iterator(
                             path.value(),
                             std::filesystem::directory_options::follow_directory_symlink |
                                 std::filesystem::directory_options::skip_permission_denied))
                    {
                        dir_paths.emplace_back(entry);
                    }
                    result.resize(dir_paths.size());
                    std::transform(std::execution::par_unseq, dir_paths.begin(), dir_paths.end(),
                                   result.begin(), [](const std::filesystem::path& current)
                                   { return data::ShareableFile(current); });
                }
            }
        }
        else
        {
            if (path.error() == "root")
            {
                auto root_paths = utils::filesystem::listRoots();
                result.resize(root_paths.size());
                std::transform(std::execution::par_unseq, root_paths.begin(), root_paths.end(),
                               result.begin(), [](const std::filesystem::path& current)
                               { return data::ShareableFile(current); });
            }
        }
        return result;
    }
} // namespace netdisk::service::http
