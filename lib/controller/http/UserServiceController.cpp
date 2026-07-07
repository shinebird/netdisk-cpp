#include "netdisk-cpp/controller/http/UserServiceController.hpp"
#include "netdisk-cpp/controller/http/Common.hpp"
#include "netdisk-cpp/controller/http/Macros.hpp"
#include "netdisk-cpp/embed_data/EmbedData.hpp"
#include "netdisk-cpp/mime_types/MimeTypes.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/http/read.hpp>

#include <filesystem>

namespace netdisk::controller::http
{
    namespace request
    {
        NETDISK_CONTROLLER_REQUEST(mainPage)
        {
            co_return co_await internal::defaultHandler<boost::beast::http::dynamic_body, true>(
                parser, stream, buffer);
        }

        NETDISK_CONTROLLER_REQUEST(indexPage)
        {
            co_return co_await internal::defaultHandler<boost::beast::http::dynamic_body, true>(
                parser, stream, buffer);
        }

        NETDISK_CONTROLLER_REQUEST(login)
        {
            co_return co_await internal::defaultHandler<boost::beast::http::dynamic_body, true>(
                parser, stream, buffer);
        }

        NETDISK_CONTROLLER_REQUEST(filePanel)
        {
            co_return co_await internal::defaultHandler<boost::beast::http::dynamic_body, true>(
                parser, stream, buffer);
        }

        NETDISK_CONTROLLER_REQUEST(downloadAndExtractPage)
        {
            co_return co_await internal::defaultHandler<boost::beast::http::dynamic_body, true>(
                parser, stream, buffer);
        }

        NETDISK_CONTROLLER_REQUEST(batchUploadFilesPage)
        {
            co_return co_await internal::defaultHandler<boost::beast::http::dynamic_body, true>(
                parser, stream, buffer);
        }
    } // namespace request
    namespace response
    {
        namespace internal
        {
            namespace
            {
                auto staticPage(core::http::Connection& connection, const std::string_view target,
                                core::http::Config& config) -> boost::cobalt::task<void>
                {
                    const auto embed_data = data::getEmbedData(target);
                    const auto mime_type = *(utils::mime_type::getMimeTypes(
                                                 std::filesystem::path{target}.extension().string())
                                                 .begin());
                    co_return co_await connection.staticBodyReplyWithETag(
                        boost::beast::http::status::ok, embed_data.first.data(),
                        embed_data.first.size(), mime_type, embed_data.second, config);
                }
            } // namespace
        } // namespace internal

        NETDISK_CONTROLLER_RESPONSE(mainPage)
        {
            co_await connection.redirectReply("/user/index", config);
        }

        NETDISK_CONTROLLER_RESPONSE(indexPage)
        {
            co_await internal::staticPage(connection, "/user/index.html", config);
        }

        NETDISK_CONTROLLER_RESPONSE(login)
        {
            co_await internal::staticPage(connection, "/user/login.html", config);
        }

        NETDISK_CONTROLLER_RESPONSE(filePanel)
        {
            co_await internal::staticPage(connection, "/user/files.html", config);
        }

        NETDISK_CONTROLLER_RESPONSE(downloadAndExtractPage)
        {
            co_await internal::staticPage(connection, "/user/downloadAndExtract.html", config);
        }

        NETDISK_CONTROLLER_RESPONSE(batchUploadFilesPage)
        {
            co_await internal::staticPage(connection, "/user/batchUploadFiles.html", config);
        }
    } // namespace response
} // namespace netdisk::controller::http
