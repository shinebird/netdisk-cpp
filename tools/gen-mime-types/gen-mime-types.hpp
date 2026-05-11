#pragma once

#include <string_view>

constexpr std::string_view mime_types_cpp_begin = "#include \"{}\"\n\n"
                                                  "namespace netdisk::data\n"
                                                  "{{\n";

constexpr std::string_view mime_types_cpp_end = "}";

constexpr std::string_view mime2ext_begin =
    "    static const std::flat_map<std::string_view, std::flat_set<std::string_view, "
    "std::less<>>, "
    "std::less<>> mime_type_to_file_extensions_map = {\n";

constexpr std::string_view ext2mime_begin =
    "    static const std::flat_map<std::string_view, std::flat_set<std::string_view, "
    "std::less<>>, "
    "std::less<>> file_extensions_to_mime_type_map = {\n";

constexpr std::string_view mime2ext_line = "        {{ \"{}\", {{ {} }} }},\n";

constexpr std::string_view mime2ext_line_map_value = "\"{}\", ";

constexpr std::string_view mime2ext_end = "    };\n";

constexpr std::string_view mime_types_h_content =
    "#include <flat_map>\n"
    "#include <flat_set>\n"
    "#include <string_view>\n\n"
    "namespace netdisk::utils::mime_type\n"
    "{{\n"
    "    auto getMimeTypes(std::string_view file_extension) -> \n"
    "        const std::flat_set<std::string_view, std::less<>>&;\n"
    "    auto getFileExtensions(std::string_view mime_type) -> \n"
    "        const std::flat_set<std::string_view, std::less<>>&;\n"
    "}}\n";

constexpr std::string_view mime_type_funcs = R"(
namespace netdisk::utils::mime_type
{
    auto getMimeTypes(std::string_view file_extension)
        -> const std::flat_set<std::string_view, std::less<>>&
    {
        static const std::flat_set<std::string_view, std::less<>> default_mime_type = {
            "application/octet-stream"};
        const auto iter = ::netdisk::data::file_extensions_to_mime_type_map.find(file_extension);
        if (iter != ::netdisk::data::file_extensions_to_mime_type_map.end())
        {
            return iter->second;
        }
        return default_mime_type;
    }

    auto getFileExtensions(std::string_view mime_type)
        -> const std::flat_set<std::string_view, std::less<>>&
    {
        return ::netdisk::data::mime_type_to_file_extensions_map.at(mime_type);
    }
} // namespace netdisk::utils::mime_type
// )";
