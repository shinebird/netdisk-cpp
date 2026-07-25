#include "netdisk-cpp/service/http/FileService.hpp"
#include "netdisk-cpp/data/ShareableFile.hpp"
#include "netdisk-cpp/utils/filesystem/FileQuery.hpp"
#include "netdisk-cpp/utils/filesystem/ListRoots.hpp"
#include "netdisk-cpp/utils/url/HTTPParamEncodings.hpp"

#include <algorithm>
#include <execution>
#include <filesystem>
#include <format>
#include <ranges>

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

    auto checkFileExists(const std::filesystem::path& path) -> bool
    {
        std::error_code error_code;
        const auto result = std::filesystem::exists(path, error_code);
        return result && (!error_code) && utils::filesystem::isRegularFile(path).value_or(false);
    }

    auto batchDownloadLinks(const std::filesystem::path& file_path, std::string_view username,
                            std::string_view token, std::string_view domain, std::uint16_t port)
        -> std::vector<std::string>
    {
        std::vector<std::filesystem::path> paths;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(
                 file_path, std::filesystem::directory_options::follow_directory_symlink |
                                std::filesystem::directory_options::skip_permission_denied))
        {
            paths.emplace_back(entry.path());
        }
        auto paths_with_index = paths | std::views::enumerate;
        std::vector<std::string> result;
        result.resize(paths.size() + 1);
        std::for_each(
            std::execution::par_unseq, paths_with_index.begin(), paths_with_index.end(),
            [domain, port, token, username, &result](const auto& elem) -> void
            {
                const auto& [index, path] = elem;
                result[index] = std::format(
                    "https://{}:{}/service/file/download?token={}&username={}&path={}&alias={}{}",
                    domain, port, token, username,
                    utils::url::encodeBase64(std::bit_cast<char*>(path.generic_u8string().data()))
                        .value_or(""),
                    index + 1, std::bit_cast<char*>(path.extension().generic_u8string().data()));
            });
        result.emplace_back(std::format(
            "https://{}:{}/service/file/"
            "batchDownload?token={}&username={}&path={}&domain={}&movePath=true",
            domain, port, token, username,
            utils::url::encodeBase64(std::bit_cast<char*>(file_path.generic_u8string().data()))
                .value_or(""),
            utils::url::encodeBase64(domain).value_or("")));

        return result;
    }

    auto moveFileCmd(const std::vector<std::filesystem::path>& paths,
                     const std::flat_set<std::filesystem::path>& dirs) -> std::vector<std::string>
    {
        std::vector<std::string> result;
        result.resize(paths.size() + dirs.size());
        std::transform(std::execution::par_unseq, dirs.begin(), dirs.end(), result.begin(),
                       [](const std::filesystem::path& path)
                       {
                           return std::format(R"(New-Item '{}' -ItemType Directory)",
                                              std::bit_cast<char*>(path.generic_u8string().data()));
                       });
        const auto paths_with_index = paths | std::views::enumerate;
        std::transform(std::execution::par_unseq, paths_with_index.begin(), paths_with_index.end(),
                       result.end(),
                       [](const auto& elem)
                       {
                           const auto& [index, path] = elem;
                           return std::format(
                               R"(Move-Item '{}{}' '{}')", index + 1,
                               std::bit_cast<char*>(path.extension().generic_u8string().data()),
                               std::bit_cast<char*>(path.generic_u8string().data()));
                       });
        return result;
    }

    auto pathMapping(const std::filesystem::path& file_path, const std::filesystem::path& path,
                     std::vector<std::filesystem::path>& paths,
                     std::flat_set<std::filesystem::path>& dirs) -> void
    {
        dirs.emplace(std::filesystem::relative(file_path, path).parent_path());
        if (!utils::filesystem::isDirectory(file_path).value_or(false))
        {
            paths.emplace_back(std::filesystem::relative(file_path, path));
        }
        for (const auto& entry : std::filesystem::directory_iterator(
                 file_path, std::filesystem::directory_options::follow_directory_symlink |
                                std::filesystem::directory_options::skip_permission_denied))
        {
            pathMapping(entry.path(), path, paths, dirs);
        }
        dirs.erase("");
    }
} // namespace netdisk::service::http
