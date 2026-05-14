#include "netdisk-cpp/controller/http/StaticFileController.hpp"
#include "netdisk-cpp/controller/http/Common.hpp"
#include "netdisk-cpp/embed_data/EmbedData.hpp"
#include "netdisk-cpp/mime_types/MimeTypes.hpp"
#include "netdisk-cpp/utils/log/formatter/boost/stacktrace/stacktrace.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/stacktrace/stacktrace.hpp>
#include <boost/url/pct_string_view.hpp>

#include <exception>
#include <filesystem>

#include <spdlog/spdlog.h>

namespace netdisk::controller::http
{
    namespace request
    {
        NETDISK_CONTROLLER_REQUEST(staticFile)
        {
            co_return co_await internal::defaultHandler<boost::beast::http::dynamic_body, true>(
                parser, stream, buffer);
        }
    } // namespace request
    namespace response
    {
        NETDISK_CONTROLLER_RESPONSE(staticFile)
        {
            static const std::string prefix = "/user/";
            boost::urls::pct_string_view original_target(match.at("path"));
            const auto decoded_target = original_target.decode();
            const auto target = prefix + decoded_target;

            try
            {
                const auto embed_data = data::getEmbedData(target);
                const auto mime_type = *(utils::mime_type::getMimeTypes(
                                             std::filesystem::path{target}.extension().string())
                                             .begin());
                co_return co_await connection.staticBodyReplyWithETag(
                    boost::beast::http::status::ok, embed_data.first.data(),
                    embed_data.first.size(), mime_type, embed_data.second, config);
            }
            catch (const std::exception& e)
            {
                boost::stacktrace::stacktrace trace;
                SPDLOG_LOGGER_INFO(spdlog::get("multi_logger"), R"(Unable to find static-file {})",
                                   target);
                SPDLOG_LOGGER_DEBUG(spdlog::get("multi_logger"),
                                    R"(Exception: {}, Stacktrace: \n{})", e.what(), trace);
                // std::println("{}", e.what());
            }
            co_return co_await connection.errorReply(boost::beast::http::status::not_found, "",
                                                     config);
        }
    } // namespace response
} // namespace netdisk::controller::http
