#pragma once

#include <format>

#include <boost/stacktrace/stacktrace.hpp>

template <>
struct std::formatter<boost::stacktrace::stacktrace>
    : public std::formatter<std::string, char>
{
            constexpr auto parse(std::format_parse_context& ctx)
            {
                // do nothing
                return ctx.begin();
            }

            template <typename FormatContext>
            auto format(const boost::stacktrace::stacktrace& trace, FormatContext& ctx) const
            {
                return std::format_to(ctx.out(), "{}", ::boost::stacktrace::to_string(trace));
            }
};
