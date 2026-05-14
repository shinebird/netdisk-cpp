#include <boost/beast/http/field.hpp>
#include <boost/program_options.hpp>


#include <spdlog/common.h>

#include <cstdint>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "CommandLine.hpp"

#include "netdisk-cpp/utils/log/Logger.hpp"
#include "netdisk-cpp/controller/http/Controller.hpp"
#include "netdisk-cpp/core/http/Config.hpp"
#include "netdisk-cpp/core/http/Server.hpp"
#include "netdisk-cpp/core/http/config/CORS.hpp"

static void printHelpMessage(const std::string_view& program_name)
{
    std::println(std::cout, "Personal Netdisk Main Application\nUsage: {} [options] <input>\n",
                 program_name);
}

auto main(int argc, char* argv[]) -> int
{
    boost::program_options::options_description desc{"Options"};
    boost::program_options::positional_options_description positional_desc;
    desc.add_options()("help,h", "Help screen")(
        "http-port", boost::program_options::value<std::uint16_t>(), "Port to listen (HTTP)")(
        "grpc-port", boost::program_options::value<std::uint16_t>(), "Port to listen (gRPC)")(
        "threads,j", boost::program_options::value<std::uint64_t>(), "Number of threads to use")(
        "file-logger-level",
        boost::program_options::value<spdlog::level::level_enum>()
            ->default_value(spdlog::level::level_enum::debug, "debug")
            ->notifier([](spdlog::level::level_enum)
                       { netdisk::tools::command_line::checkCustomType("file-logger-level"); }),
        "Log level of file-logger [trace, debug, info, warn, err, critical, off]")(
        "console-logger-level",
        boost::program_options::value<spdlog::level::level_enum>()
            ->default_value(spdlog::level::level_enum::warn, "warn")
            ->notifier([](spdlog::level::level_enum)
                       { netdisk::tools::command_line::checkCustomType("file-logger-level"); }),
        "Log level of console-logger [trace, debug, info, warn, err, critical, off]");
    // positional_desc.add("input", -1);
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
        std::println("An error occoured while parsing commmand-line: {}", e.what());
        throw e;
        return -1;
    }

    if (vm.contains("help") || vm.empty())
    {
        printHelpMessage(argv[0]);
        std::cout << "\n\n" << desc << '\n';
        return 1;
    }

    netdisk::core::http::Server server(vm.at("http-port").as<std::uint16_t>(),
                                       vm.at("threads").as<std::uint64_t>());

    netdisk::core::http::config::CORSRegistration cors_registration;
    cors_registration.setAllowHeaders(boost::beast::http::field::unknown);
    cors_registration.setAllowMethods(boost::beast::http::verb::get, boost::beast::http::verb::post,
                                      boost::beast::http::verb::delete_,
                                      boost::beast::http::verb::put,
                                      boost::beast::http::verb::options);
    cors_registration.setAllowOrigin("*");
    cors_registration.setMaxAge(3600);
    server.getConfig().getCORS().addMapping("/{path+}", std::move(cors_registration));

    netdisk::utils::log::Logger logger;
    logger.setConsoleLogLevel(vm.at("console-logger-level").as<spdlog::level::level_enum>());
    logger.setFileLogLevel(vm.at("file-logger-level").as<spdlog::level::level_enum>());
    logger.init();
    server.setLogger(logger.getLogger());

    netdisk::controller::http::registerAll(server);

    server.initSSL();
    server.run();

    return 0;
}
