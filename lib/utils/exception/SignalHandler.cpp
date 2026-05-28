#include <boost/stacktrace.hpp>
#include <boost/type_index.hpp>

#include "netdisk-cpp/utils/exception/SignalHandler.hpp"
#include "netdisk-cpp/utils/log/Logger.hpp"
#include "netdisk-cpp/utils/log/formatter/boost/stacktrace/stacktrace.hpp"

#include <csignal>
#include <spdlog/spdlog.h>

namespace netdisk::utils::exception::signal
{
    auto toString(int signal) -> std::string
    {
        switch (signal)
        {
        case SIGSEGV:
            return "segment violation";
        case SIGILL:
            return "illegal instruction - invalid function image";
        case SIGFPE:
            return "floating point exception";
        case SIGINT:
            return "interrupt";
        default:
            return "";
        }
    }

    void terminateHandler()
    {
        try
        {
            std::rethrow_exception(std::current_exception());
        }
        catch (const std::exception& e)
        {
            SPDLOG_LOGGER_ERROR(spdlog::get("multi_logger"), "The last exception was {}: {}",
                                boost::typeindex::type_id_runtime(e).pretty_name(), e.what());
        }
        try
        {
            SPDLOG_LOGGER_ERROR(
                spdlog::get("multi_logger"),
                "Program crashed, see the following stack trace for more infomation: \n\n{}",
                boost::stacktrace::stacktrace());
        }
        catch (...)
        {
            spdlog::get("multi_logger")->flush();
        }
        std::abort();
    }
} // namespace netdisk::utils::exception::signal
