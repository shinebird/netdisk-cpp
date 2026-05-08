#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <print>
#include <string>
#include <string_view>

#include <boost/program_options.hpp>

#include "gen-embed.hpp"

static void printHelpMessage(const std::string_view& program_name)
{
    std::println(std::cout,
                 "c++26 embed-based source file generator\nUsage: {} [options] <input>\n",
                 program_name);
}

auto main(int argc, char* argv[]) -> int
{
    boost::program_options::options_description desc{"Options"};
    desc.add_options()("help,h", "Help screen")(
        "base-path", boost::program_options::value<std::string>(), "The base path to the data")(
        "output,o", boost::program_options::value<std::string>(),
        "Path to .cpp output, .hpp output will be generated in the same directory");
    boost::program_options::command_line_parser command_line_parser{argc, argv};
    command_line_parser.options(desc).style(
        boost::program_options::command_line_style::default_style |
        boost::program_options::command_line_style::allow_slash_for_short |
        boost::program_options::command_line_style::case_insensitive);
    boost::program_options::variables_map vm;
    try
    {
        boost::program_options::parsed_options parsed_options = command_line_parser.run();
        boost::program_options::store(parsed_options, vm);
        boost::program_options::notify(vm);
    }
    catch (const boost::program_options::error& e)
    {
        std::println("{}", e.what());
        return -1;
    }

    if (vm.contains("help") || vm.empty())
    {
        printHelpMessage(argv[0]);
        std::cout << "\n\n" << desc << '\n';
        return 1;
    }

    // std::filesystem::path path_to_folder = R"(D:\Software\Tools\netdisk-cpp\cpp_embed)";

    std::filesystem::path path_to_folder = vm["base-path"].as<std::string>();
    if (!(std::filesystem::exists(path_to_folder) && std::filesystem::is_directory(path_to_folder)))
    {
        std::println(std::cerr, "[ERROR] Incorrect base path \"{}\"",
                     vm["base-path"].as<std::string>());
        return -1;
    }
    const auto embed_data_cpp_path = vm["output"].as<std::string>();
    std::filesystem::path embed_data_cpp_fs_path = embed_data_cpp_path;
    std::ofstream embed_data_cpp_stream(embed_data_cpp_path, std::ios::binary | std::ios::out);
    std::ofstream embed_data_h_stream(
        embed_data_cpp_path.subview(0, embed_data_cpp_path.size() - 3) + std::string("hpp"),
        std::ios::binary | std::ios::out);
    std::println(
        embed_data_cpp_stream, embed_cpp_begin,
        std::bit_cast<char* const>(
            std::filesystem::absolute(path_to_folder).generic_u8string().data()),
        std::bit_cast<char* const>(
            (embed_data_cpp_fs_path.stem().generic_u8string() + std::u8string(u8".hpp")).data()));
    std::uint64_t index = 0;
    std::list<std::string> embed_data_paths;
    std::set<uint64_t> empty_file_indices;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path_to_folder))
    {
        if (entry.is_regular_file())
        {
            embed_data_paths.emplace_back(std::string("/").append(std::bit_cast<char* const>(
                entry.path().lexically_relative(path_to_folder).generic_u8string().data())));
            std::println(embed_data_cpp_stream, embed_line_format, index,
                         std::bit_cast<char* const>(
                             std::filesystem::absolute(entry.path()).generic_u8string().data()));
            if (entry.file_size() == 0) [[unlikely]]
            {
                empty_file_indices.emplace(index);
            }
            index++;
        }
    }
    /// due to missing __cpp_lib_constexpr_map or __cpp_lib_constexpr_flat_map
    std::println(
        embed_data_cpp_stream,
        "    static const std::flat_map<std::string_view, std::span<const char>> embed_data = {{");
    index = 0;
    for (const auto& entry : embed_data_paths)
    {
        if (empty_file_indices.contains(index)) [[unlikely]]
        {
            std::println(embed_data_cpp_stream, "        {{ R\"({})\", {{}} }},", entry);
        }
        else
        {
            std::println(embed_data_cpp_stream, "        {{ R\"({})\", EMBED_SOURCE_{} }},", entry,
                         index);
        }

        index++;
    }
    std::println(embed_data_cpp_stream, "    }};\n");
    std::println(embed_data_cpp_stream,
                 "    auto getEmbedData(const std::string_view path) -> std::span<const char>\n"
                 "    {{\n"
                 "        return embed_data.at(path);\n"
                 "    }}");
    std::println(embed_data_cpp_stream, "}}");

    std::println(embed_data_h_stream, embed_h_content,
                 std::bit_cast<char* const>(
                     std::filesystem::absolute(path_to_folder).generic_u8string().data()));
    return 0;
}

// namespace netdisk
