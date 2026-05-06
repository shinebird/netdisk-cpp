#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/parser/parser.hpp>
#include <boost/program_options.hpp>

#include <filesystem>
#include <flat_map>
#include <flat_set>
#include <fstream>
#include <functional>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "gen-mime-types.h"
#include <netdisk-cpp/utils/string/StringUtils.hpp>


static void printHelpMessage(const std::string_view& program_name)
{
    std::println(std::cout, "Mime-Type CSV to C++ file generator\nUsage: {} [options] <input>\n",
                 program_name);
}

auto main(int argc, char* argv[]) -> int
{
    boost::program_options::options_description desc{"Options"};
    boost::program_options::positional_options_description positional_desc;
    desc.add_options()("help,h", "Help screen")(
        "input", boost::program_options::value<std::string>(), "Mime-type CSV input")(
        "output,o", boost::program_options::value<std::string>(),
        "Path to .cpp output, .h output will be generated in the same directory");
    positional_desc.add("input", -1);
    boost::program_options::command_line_parser command_line_parser{argc, argv};
    command_line_parser.options(desc)
        .style(boost::program_options::command_line_style::default_style |
               boost::program_options::command_line_style::allow_slash_for_short |
               boost::program_options::command_line_style::case_insensitive)
        .positional(positional_desc);
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

    const auto input_path = vm["input"].as<std::string>();
    boost::iostreams::mapped_file_source input_buffer(input_path);
    input_buffer.close();
    input_buffer.open(input_path);
    boost::iostreams::stream<boost::iostreams::mapped_file_source> input_stream(input_buffer);

    std::string line;
    std::getline(input_stream, line); // Ignore the csv header
    std::vector<std::string> mime_type_list;
    std::vector<std::string> file_extension_list;
    std::unordered_set<std::string, netdisk::utils::string::Hash, std::equal_to<>> mime_type_set;
    std::unordered_set<std::string, netdisk::utils::string::Hash, std::equal_to<>>
        file_extension_set;
    std::size_t mime_type_count = 0;
    std::size_t file_extension_count = 0;
    bool duplicate_mime_type = false;
    bool duplicate_file_extension = false;
    auto const action_mime_type = [&](auto& ctx) -> void
    {
        mime_type_list.emplace_back(_attr(ctx));
        mime_type_set.emplace(_attr(ctx));
        duplicate_mime_type = (mime_type_set.size() == mime_type_count);
        mime_type_count = mime_type_set.size();
    };
    auto const action_file_extension = [&](auto& ctx) -> void
    {
        file_extension_list.emplace_back(_attr(ctx));
        if (!_attr(ctx).empty())
        {
            file_extension_set.emplace(_attr(ctx));
            duplicate_file_extension = (file_extension_set.size() == file_extension_count);
            file_extension_count = file_extension_set.size();
            // if (duplicate_file_extension)
            // {
            //     std::println(std::cout, "{}", _attr(ctx));
            // }
        }
        else
        {
            duplicate_file_extension = false;
        }
    };
    std::size_t line_no = 2;
    bool error_occoured = false;
    while (std::getline(input_stream, line))
    {
        namespace bp = boost::parser;
        const auto result =
            boost::parser::parse(line,
                                 bp::quoted_string[action_mime_type] >> *(bp::ws) >> ',' >>
                                     *(bp::ws) >> bp::quoted_string[action_file_extension],
                                 bp::ws);
        if (!result)
        {
            std::println(std::cerr, "ERROR on line {}: wrong format", line_no);
            error_occoured = true;
        }
        if (duplicate_mime_type)
        {
            std::println(std::cerr, "ERROR on line {}: duplicate mime type", line_no);
            duplicate_mime_type = false;
            error_occoured = true;
        }
        // if (duplicate_file_extension)
        // {
        //     std::println(std::cerr, "WARNING on line {}: duplicate file extension", line_no);
        //     duplicate_file_extension = false;
        // }
        ++line_no;
    }
    input_stream.close();
    if (error_occoured)
    {
        std::println(std::cerr, "Error(s) occoured while generating mime-types cpp files.");
        return 1;
    }
    // static const std::flat_map<std::string_view, std::flat_set<std::string_view, std::less<>>,
    //                            std::less<>>
    //     tmp = {
    //         {"mime-type1",
    //          {
    //          "file-ext1",
    //          }},
    //         {"mime-type2",
    //          {
    //          "file-ext1",
    //          "file-ext2",
    //          }},
    // };

    const auto mime_types_cpp_path = vm["output"].as<std::string>();
    std::filesystem::path mime_types_cpp_fs_path = mime_types_cpp_path;
    std::ofstream mime_types_cpp_stream(mime_types_cpp_path, std::ios::binary | std::ios::out);
    std::ofstream mime_types_h_stream(
        mime_types_cpp_path.subview(0, mime_types_cpp_path.size() - 3) + std::string("h"),
        std::ios::binary | std::ios::out);
    std::println(mime_types_h_stream, mime_types_h_content);
    std::println(
        mime_types_cpp_stream, mime_types_cpp_begin,
        std::bit_cast<char* const>(
            (mime_types_cpp_fs_path.stem().generic_u8string() + std::u8string(u8".h")).data()));
    std::string mime2ext_str(mime2ext_begin);
    std::string ext2mime_str(ext2mime_begin);
    using FlatStringViewSet = std::flat_set<std::string_view, std::less<>>;
    std::flat_map<std::string, FlatStringViewSet, std::less<>>
        file_extensions_to_mime_type_map;
    // file_extensions_to_mime_type_map.try_emplace("current_file_extension", FlatStringViewSet{std::string_view("mime_type")});

    for (const auto&& [mime_type, file_extensions] :
         std::views::zip(mime_type_list, file_extension_list))
    {
        namespace bp = boost::parser;
        std::string mime2ext_line_map_value_str;
        const auto gen_cpp_text = [&](auto& ctx) -> void
        {
            const auto current_file_extension = _attr(ctx);
            mime2ext_line_map_value_str += std::format(mime2ext_line_map_value, current_file_extension); 
            if (!current_file_extension.empty())
            {
                // std::println("Inserting: {}", current_file_extension);
                auto [iter, inserted] = file_extensions_to_mime_type_map.try_emplace(current_file_extension, FlatStringViewSet{mime_type});
                // std::println("Inserting: {}", iter->first);
                if (!inserted)
                {
                   iter->second.emplace(mime_type); 
                }
            }
        };
        // std::println("size: {}", file_extensions_to_mime_type_map.size());
        const auto file_extension_rule = +(bp::char_ - ',');
        const auto result = bp::parse(
            file_extensions,
            (file_extension_rule[gen_cpp_text] % ((*bp::ws) >> ',' >> (*bp::ws))) | bp::eps,
            bp::ws);
        if (!result)
        {
            std::println(std::cerr, "ERROR while parsing file extension(s) for mime-type \"{}\"",
                         mime_type);
            error_occoured = true;
        }
        std::string mime2ext_line_str =
            std::format(mime2ext_line, mime_type, mime2ext_line_map_value_str);
        mime2ext_str += mime2ext_line_str;
    }
    mime2ext_str += mime2ext_end;

    for (const auto&[file_extension, mime_types] : file_extensions_to_mime_type_map)
    {
        // std::println("{}: ", file_extension);
        std::string ext2mime_line_map_value_str;
        for (const auto& current_mime_type : mime_types)
        {
            // std::println("    {}", current_mime_type);
            ext2mime_line_map_value_str += std::format(mime2ext_line_map_value, current_mime_type); 
        }
        std::string ext2mime_line_str =
            std::format(mime2ext_line, file_extension, ext2mime_line_map_value_str);
        ext2mime_str += ext2mime_line_str;
    }
    ext2mime_str += mime2ext_end;

    std::println(mime_types_cpp_stream, "{}", mime2ext_str);
    std::println(mime_types_cpp_stream, "{}", ext2mime_str);

    std::println(mime_types_cpp_stream, "{}", mime_types_cpp_end);

    std::println(mime_types_cpp_stream, "{}", mime_type_funcs);

    if (error_occoured)
    {
        std::println(std::cerr, "Error(s) occoured while generating mime-types cpp files.");
        return 1;
    }

    return 0;
}
