#include "netdisk-cpp/data/ShareableFile.hpp"
#include "netdisk-cpp/utils/filesystem/FileQuery.hpp"

#include <boost/json.hpp>
#include <exception>
#include <filesystem>

namespace netdisk::data
{
    ShareableFile::ShareableFile(const std::filesystem::path& path)
        : file_size_(utils::filesystem::getFileSize(path).value_or(0)), file_name_(path.filename()),
          absolute_path_(std::filesystem::absolute(path)),
          is_directory_(utils::filesystem::isDirectory(path).value_or(false)),
          last_modified_(utils::filesystem::lastWriteTime(path).value_or(0LL))
    {
    }

    void tag_invoke(const boost::json::value_from_tag&, boost::json::value& json_value,
                    const ShareableFile& shareable_file)
    {
        boost::json::object result = {
            {    "fileSize",                shareable_file.file_size_                            },
            {    "fileName",
             std::bit_cast<char* const>(shareable_file.file_name_.generic_u8string().c_str())    },
            {"absolutePath",
             std::bit_cast<char* const>(shareable_file.absolute_path_.generic_u8string().c_str())},
            { "isDirectory",                                         shareable_file.is_directory_},
            {"lastModified",                                        shareable_file.last_modified_},
        };
        json_value = result;
    }
} // namespace netdisk::data
