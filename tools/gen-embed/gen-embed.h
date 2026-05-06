#pragma once

#include <string_view>

constexpr std::string_view embed_cpp_begin = "/// Base-Path: {}\n\n\n"
                                             "#include <flat_map>\n"
                                             "#include \"{}\"\n\n"
                                             "namespace netdisk::data\n"
                                             "{{";

constexpr std::string_view embed_line_format = "    static constexpr char EMBED_SOURCE_{}[] = {{\n"
                                               "    #embed \"{}\"\n"
                                               "    }};\n";

constexpr std::string_view embed_h_content =
    "/// Base-Path: {}\n\n\n"
    "#include <span>\n"
    "#include <string_view>\n\n"
    "namespace netdisk::data\n"
    "{{\n"
    "    auto getEmbedData(const std::string_view path) -> std::span<const char>;\n"
    "}}";
