#include "netdisk-cpp/controller/http/UserServiceController.hpp"
#include "netdisk-cpp/controller/http/Common.hpp"
#include "netdisk-cpp/controller/http/Macros.hpp"
#include "netdisk-cpp/embed_data/EmbedData.hpp"
#include "netdisk-cpp/mime_types/MimeTypes.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/http/read.hpp>

#include <filesystem>

namespace netdisk::core::http::controller
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

        NETDISK_CONTROLLER_REQUEST(downloadAndExtract)
        {
            co_return co_await internal::defaultHandler<boost::beast::http::dynamic_body, true>(
                parser, stream, buffer);
        }

        NETDISK_CONTROLLER_REQUEST(batchUploadFiles)
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
                auto staticPage(Connection& connection, const std::string_view target,
                                Config& config) -> boost::asio::awaitable<void>
                {
                    const auto embed_data = data::getEmbedData(target);
                    const auto mime_type = *(utils::mime_type::getMimeTypes(
                                                 std::filesystem::path{target}.extension().string())
                                                 .begin());
                    co_return co_await connection.staticBodyReply(
                        boost::beast::http::status::ok, embed_data.data(), embed_data.size(),
                        mime_type, config);
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

        NETDISK_CONTROLLER_RESPONSE(downloadAndExtract)
        {
            co_await internal::staticPage(connection, "/user/downloadAndExtract.html", config);
        }

        NETDISK_CONTROLLER_RESPONSE(batchUploadFiles)
        {
            co_await internal::staticPage(connection, "/user/batchUploadFiles.html", config);
        }
    } // namespace response
} // namespace netdisk::core::http::controller
