#include "netdisk-cpp/controller/http/FileController.hpp"
#include "netdisk-cpp/controller/http/Common.hpp"
#include "netdisk-cpp/mime_types/MimeTypes.hpp"
#include "netdisk-cpp/service/http/FileService.hpp"
#include "netdisk-cpp/utils/filesystem/FileQuery.hpp"
#include "netdisk-cpp/utils/string/StringUtils.hpp"
#include "netdisk-cpp/utils/url/HTTPParamEncodings.hpp"

#include <expected>
#include <filesystem>
#include <flat_map>
#include <optional>
#include <string>

#include <boost/beast/http/field.hpp>
#include <boost/json.hpp>
#include <boost/url.hpp>

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

        NETDISK_CONTROLLER_REQUEST(batchDownloadFile)
        {
            const auto target = parser.get().target();
            const boost::urls::url_view url_view = boost::urls::parse_uri_reference(target).value();
            const auto params = url_view.params();
            const auto username = getParam(params, "username");
            const auto token = getParam(params, "token");
            const auto path = getParam(params, "path");
            const auto domain = getParam(params, "domain");
            const auto move_path = getParam(params, "movePath");
            if (!hasAllRequestParams(username, token, path, domain))
            {
                extra_data = false;
            }
            else
            {
                const auto decoded_path = utils::url::decodeBase64(path).value_or("");
                const auto decoded_domain = utils::url::decodeBase64(domain).value_or("");
                bool valid_params = hasAllRequestParams(decoded_path, decoded_domain);
                if (!valid_params)
                {
                    extra_data = false;
                }
                else
                {
                    const std::flat_map<std::string, std::string> result = {
                        { "username",       username},
                        {    "token",          token},
                        {     "path",   decoded_path},
                        {   "domain", decoded_domain},
                        {"move_path",      move_path},
                    };
                    extra_data = result;
                }
            }
            co_return pro::make_proxy<core::http::proxy::Request>(std::move(parser.get()));
        }
    } // namespace request

    namespace response
    {
        NETDISK_CONTROLLER_RESPONSE(getShareableFiles)
        {
            if (!extra_data.has_value())
            {
                std::string_view msg = "400 Bad Request";
                co_return co_await connection.errorReply(boost::beast::http::status::bad_request,
                                                         msg, config);
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
                co_return co_await connection.errorReply(boost::beast::http::status::bad_request,
                                                         msg, config);
            }
            bool file_exists = service::http::checkFileExists(path.value());
            boost::json::value json_value = file_exists;
            const auto json_string = boost::json::serialize(json_value);
            co_return co_await connection.staticBodyReply(boost::beast::http::status::ok,
                                                          json_string, json_string.size(),
                                                          "application/json", config);
        }

        NETDISK_CONTROLLER_RESPONSE(batchDownloadFile)
        {
            if (std::any_cast<bool>(&extra_data) != nullptr)
            {
                std::string_view msg = "400 Bad Request";
                co_return co_await connection.errorReply(boost::beast::http::status::bad_request,
                                                         msg, config);
            }
            const auto& params =
                std::any_cast<std::flat_map<std::string, std::string>&>(extra_data);
            std::filesystem::path file_path = params.at("path");
            if (!(std::filesystem::exists(file_path) &&
                  utils::filesystem::isDirectory(file_path).value_or(false)))
            {
                std::string_view msg = "400 Bad Request";
                co_return co_await connection.errorReply(boost::beast::http::status::bad_request,
                                                         msg, config);
            }
            if (params.at("move_path") == "true")
            {
                std::vector<std::filesystem::path> paths;
                std::flat_set<std::filesystem::path> dirs;
                service::http::pathMapping(file_path, file_path, paths, dirs);
                auto commands = service::http::moveFileCmd(paths, dirs);
                auto content = utils::string::joinWithNewline(commands);
                boost::beast::http::fields extra_fields;
                extra_fields.insert(
                    boost::beast::http::field::content_disposition,
                    std::format("attachment; {}",
                                utils::url::encodeContentDispositionFileName("move_path.ps1")));
                co_return co_await connection.staticBodyReply(
                    boost::beast::http::status::ok, content, content.size(),
                    *utils::mime_type::getMimeTypes(".ps1").begin(), config, extra_fields);
            }
            else
            {
                auto links = service::http::batchDownloadLinks(
                    file_path, params.at("username"), params.at("token"), params.at("domain"),
                    config.getPort());
                auto content = utils::string::joinWithNewline(links);
                boost::beast::http::fields extra_fields;
                extra_fields.insert(
                    boost::beast::http::field::content_disposition,
                    std::format("attachment; {}", utils::url::encodeContentDispositionFileName(
                                                      "batch_download_link.txt")));
                co_return co_await connection.staticBodyReply(
                    boost::beast::http::status::ok, content, content.size(),
                    *utils::mime_type::getMimeTypes(".txt").begin(), config, extra_fields);
            }
        }
    } // namespace response
} // namespace netdisk::controller::http
