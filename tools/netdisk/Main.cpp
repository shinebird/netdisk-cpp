#include <boost/beast/http/field.hpp>
#include <boost/program_options.hpp>

#include <cstdint>
#include <iostream>
#include <print>
#include <string>
#include <string_view>

#include "CommandLine.hpp"

#include "netdisk-cpp/controller/generic/security/UserAuthenticator.hpp"
#include "netdisk-cpp/controller/http/Controller.hpp"
#include "netdisk-cpp/controller/http/security/AuthorizationManager.hpp"
#include "netdisk-cpp/core/http/Config.hpp"
#include "netdisk-cpp/core/http/Server.hpp"
#include "netdisk-cpp/core/http/config/CORS.hpp"
#include "netdisk-cpp/utils/log/Logger.hpp"

#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE
    #include "netdisk-cpp/repository/sqlite/DataBaseConnection.hpp"
#endif

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
        "grpc-port", boost::program_options::value<std::uint16_t>(),
        "Port to listen (gRPC)")("threads,j",
                                 boost::program_options::value<std::uint64_t>()->default_value(
                                     std::thread::hardware_concurrency(),
                                     std::format("{}", std::thread::hardware_concurrency())),
                                 "Number of threads to use")(
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
        "Log level of console-logger [trace, debug, info, warn, err, critical, off]")(
        "log-file-path",
        boost::program_options::value<std::string>()->default_value("log.txt", "log.txt"),
        "Path to log file")
#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE
        ("database-path", boost::program_options::value<std::string>(), "Path to sqlite database")
#endif
        ;
    // positional_desc.add("input", -1);
    boost::program_options::command_line_parser command_line_parser{argc, argv};
    command_line_parser.options(desc)
        .style(boost::program_options::command_line_style::default_style |
               boost::program_options::command_line_style::allow_slash_for_short |
               boost::program_options::command_line_style::case_insensitive)
        .positional(positional_desc);
    boost::program_options::variables_map vm;
    bool no_option = false;
    try
    {
        boost::program_options::parsed_options parsed_options = command_line_parser.run();
        no_option = parsed_options.options.empty();
        boost::program_options::store(parsed_options, vm);
        boost::program_options::notify(vm);
    }
    catch (const boost::program_options::error& e)
    {
        std::println("An error occoured while parsing commmand-line: {}", e.what());
        throw e;
    }

    if (vm.contains("help") || no_option)
    {
        printHelpMessage(argv[0]);
        std::cout << "\n\n" << desc << '\n';
        return 1;
    }

#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE
    netdisk::repository::database::sqlite::Connection database_connection(
        vm.at("database-path").as<std::string>());
#endif
    netdisk::controller::security::UserAuthenticator user_authenticator;
    netdisk::controller::http::security::AuthorizationManager authorization_manager;

    netdisk::core::http::Server server(
        vm.at("http-port").as<std::uint16_t>(), vm.at("threads").as<std::uint64_t>(),
#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE

        std::addressof(database_connection),
#endif
        std::addressof(user_authenticator), std::addressof(authorization_manager));

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
    logger.setLogPath(vm.at("log-file-path").as<std::string>());
    logger.init();
    server.setLogger(logger.getLogger());

    netdisk::controller::http::registerAll(server);

    server.initSSL();
    server.run();

    spdlog::shutdown();

    return 0;
}
