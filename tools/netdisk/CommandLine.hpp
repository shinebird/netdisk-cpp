#pragma once

#include <flat_map>
#include <string_view>

#include <boost/program_options.hpp>

namespace spdlog::level
{
    enum level_enum : int;

    void validate(boost::any& v, const std::vector<std::string>& values, level_enum*,
              int);
}

namespace netdisk::tools::command_line
{
    /// status: 1=success 0=fail
    // extern std::flat_map<std::string_view, char, std::less<>> cmd_parse_status;
    struct ParseState
    {
            bool success_ = true;
            boost::program_options::validation_error::kind_t error_state_;
            std::string original_token_;
    };

    struct LogLevel
    {
        spdlog::level::level_enum level_;
    };

    auto getParseState() -> ParseState;

    auto checkCustomType(std::string_view option_name) -> void;
} // namespace netdisk::tools::command_line


