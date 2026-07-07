#include "netdisk-cpp/controller/http/FileController.hpp"
#include "netdisk-cpp/service/http/FileService.hpp"

#include <expected>
#include <filesystem>
#include <optional>
#include <string>

#include <boost/asio/use_awaitable.hpp>
#include <boost/json.hpp>

#include <spdlog/spdlog.h>

namespace netdisk::controller::http
{
    namespace request
    {
        NETDISK_CONTROLLER_REQUEST(getShareableFiles)
        {
            boost::beast::http::request_parser<boost::beast::http::string_body> new_parser{
                std::move(parser)};
            co_await boost::beast::http::async_read(stream, buffer, new_parser);
            const auto& body = new_parser.get().body();
            boost::json::stream_parser json_parser;
            json_parser.reset();
            json_parser.write(body);
            json_parser.finish();
            const auto json_value = json_parser.release();
            if (const auto& path = json_value.try_at("path")->try_as_string())
            {
                std::expected<std::filesystem::path, std::string> result;
                if (path->size() <= 1 || path.value() == "root")
                {
                    result = std::unexpected("root");
                }
                else
                {
                    std::string new_path = path->c_str();
#ifdef _WIN32
                    if (new_path.ends_with(':'))
                    {
                        new_path += "/";
                    }
#endif
                    std::filesystem::path fs_path(new_path);
                    if (fs_path.is_absolute())
                    {
                        result = fs_path;
                    }
                    else
                    {
                        result = std::unexpected("root");
                    }
                }
                extra_data = result;
            }
            else
            {
                SPDLOG_LOGGER_DEBUG(
                    spdlog::get("multi_logger"),
                    "[POST] /service/file/listFiles: Invalid JSON body received: {}", body.c_str());
            }
            co_return pro::make_proxy<core::http::proxy::Request>(std::move(new_parser.get()));
        }

        NETDISK_CONTROLLER_REQUEST(checkFileExists)
        {
            boost::beast::http::request_parser<boost::beast::http::string_body> new_parser{
                std::move(parser)};
            co_await boost::beast::http::async_read(stream, buffer, new_parser);
            const auto& body = new_parser.get().body();
            boost::json::stream_parser json_parser;
            json_parser.reset();
            json_parser.write(body);
            json_parser.finish();
            const auto json_value = json_parser.release();
            std::optional<std::filesystem::path> fs_path = std::nullopt;
            if (const auto& path = json_value.try_at("path")->try_as_string())
            {
                std::string new_path = path->c_str();
#ifdef _WIN32
                if (new_path.ends_with(':'))
                {
                    new_path += "/";
                }
#endif
                fs_path.emplace(new_path);
            }
            else
            {
                SPDLOG_LOGGER_DEBUG(
                    spdlog::get("multi_logger"),
                    "[POST] /service/file/checkFileExists: Invalid JSON body received: {}",
                    body.c_str());
            }
            extra_data = fs_path;
            co_return pro::make_proxy<core::http::proxy::Request>(std::move(new_parser.get()));
        }

        // NETDISK_CONTROLLER_REQUEST(batchDownloadFile) {}
    } // namespace request

    namespace response
    {
        NETDISK_CONTROLLER_RESPONSE(getShareableFiles)
        {
            if (!extra_data.has_value())
            {
                std::string_view msg = "400 Bad Request";
                co_return co_await connection.staticBodyReply(
                    boost::beast::http::status::bad_request, msg, msg.size(), "text/plain", config);
            }
            const auto& path =
                std::any_cast<std::expected<std::filesystem::path, std::string>&>(extra_data);
            auto shareable_files = service::http::getShareableFiles(path);
            const auto json_string =
                boost::json::serialize(boost::json::value_from(shareable_files));
            co_return co_await connection.staticBodyReply(boost::beast::http::status::ok,
                                                          json_string, json_string.size(),
                                                          "application/json", config);
        }

        NETDISK_CONTROLLER_RESPONSE(checkFileExists)
        {
            const auto& path = std::any_cast<std::optional<std::filesystem::path>&>(extra_data);
            if (!path)
            {
                std::string_view msg = "400 Bad Request";
                co_return co_await connection.staticBodyReply(
                    boost::beast::http::status::bad_request, msg, msg.size(), "text/plain", config);
            }
            bool file_exists = service::http::checkFileExists(path.value());
            boost::json::value json_value = file_exists;
            const auto json_string = boost::json::serialize(json_value);
            co_return co_await connection.staticBodyReply(boost::beast::http::status::ok,
                                                          json_string, json_string.size(),
                                                          "application/json", config);
        }

        // NETDISK_CONTROLLER_RESPONSE(batchDownloadFile) {}
    } // namespace response
} // namespace netdisk::controller::http
