#include <boost/program_options.hpp>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <flat_map>
#include <string>
#include <string_view>
#include <print>

#include "CommandLine.hpp"
#include "netdisk-cpp/utils/reflect/Enum.hpp"

namespace netdisk::tools::command_line
{
    static const std::flat_map<std::string_view, spdlog::level::level_enum, std::less<>>
        valid_log_levels = {
            {   "trace",    spdlog::level::trace},
            {   "debug",    spdlog::level::debug},
            {    "info",     spdlog::level::info},
            {    "warn",     spdlog::level::warn},
            {     "err",      spdlog::level::err},
            {"critical", spdlog::level::critical},
            {     "off",      spdlog::level::off},
    };

    static ParseState parse_state_;
    // std::flat_map<std::string_view, char, std::less<>> cmd_parse_status
    // {
    //     {"help", 1},
    //     {"http-port", 1},
    //     {"grpc-port", 1},
    //     {"threads", 1},
    //     {"file-logger-level", 1},
    //     {"console-logger-level", 1},
    // };

    auto getParseState() -> ParseState { return parse_state_; }

    auto checkCustomType(std::string_view option_name) -> void
    {
        const auto result = getParseState();
        if (!result.success_)
        {
            std::println("custom command-line error");
            throw boost::program_options::validation_error(result.error_state_, option_name.data(),
                                                           result.original_token_);
        }
    }
} // namespace netdisk::tools::command_line

void spdlog::level::validate(boost::any& v, const std::vector<std::string>& values, level_enum*,
                             int)
{
    // Make sure no previous assignment to 'a' was made.
    boost::program_options::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const auto& current_token = boost::program_options::validators::get_single_string(values);
    auto iter = netdisk::tools::command_line::valid_log_levels.find(current_token);
    if (iter != netdisk::tools::command_line::valid_log_levels.end())
    {
        v = boost::any(iter->second);
        netdisk::tools::command_line::parse_state_.success_ = true;
    }
    else
    {
        netdisk::tools::command_line::parse_state_ = {
            .success_ = false,
            .error_state_ = boost::program_options::validation_error::invalid_option_value,
            .original_token_ = current_token,
        };
    }
}
