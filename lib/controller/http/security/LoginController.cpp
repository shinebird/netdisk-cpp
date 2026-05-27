#include "netdisk-cpp/controller/http/security/LoginController.hpp"
#include "netdisk-cpp/controller/generic/security/UserAuthenticator.hpp"
#include "netdisk-cpp/controller/http/Common.hpp"
#include "netdisk-cpp/utils/jwt/JWT.hpp"

#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE
    #include "netdisk-cpp/repository/sqlite/UserRepository.hpp"
#endif

#include <boost/beast/http/string_body.hpp>
#include <boost/json.hpp>

#include <spdlog/spdlog.h>

namespace netdisk::repository
{
#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE
    namespace impl = sqlite;
#endif
} // namespace netdisk::repository

namespace netdisk::controller::http::security
{
    namespace internal
    {
        struct LoginStatus
        {
                std::string user_name_;
                std::string token_;
                bool success_{};
        };
    } // namespace internal

    namespace request
    {
        NETDISK_CONTROLLER_REQUEST(login)
        {
            boost::beast::http::request_parser<boost::beast::http::string_body> new_parser{
                std::move(parser)};
            co_await boost::beast::http::async_read(stream, buffer, new_parser,
                                                    boost::asio::use_awaitable);

            const auto& body = new_parser.get().body();
            boost::json::stream_parser json_parser;
            json_parser.reset();
            json_parser.write(body);
            json_parser.finish();
            const auto json_value = json_parser.release();
            const auto user_name_value = json_value.try_at("username")->try_as_string();
            const auto password_value = json_value.try_at("password")->try_as_string();
            bool valid_user = false;
            internal::LoginStatus login_status;
            if (user_name_value && password_value)
            {
                const auto user = repository::impl::queryUserByUsername(
                    user_name_value->c_str(), config.getDatabaseConnection());
                if (user.getId() == data::User::invalid_id_)
                {
                    SPDLOG_LOGGER_INFO(spdlog::get("multi_logger"), "User login: Invalid id \"{}\"",
                                       user.getId());
                    valid_user = false;
                }
                else // username is valid
                {
                    if (user.getPassword() != password_value->c_str()) // password is invalid
                    {
                        SPDLOG_LOGGER_INFO(spdlog::get("multi_logger"),
                                           "User login: Invalid password for user \"{}\"",
                                           user_name_value->c_str());
                        valid_user = false;
                    }
                    else // password is valid
                    {
                        valid_user = true;
                    }
                }
                if (valid_user)
                {
                    if (const auto token = utils::jwt::createUserToken(user_name_value->c_str()))
                    {
                        config.getUserAuthenticator()->loginUser(user_name_value->c_str(),
                                                                 token.value());
                        SPDLOG_LOGGER_INFO(spdlog::get("multi_logger"),
                                           "User login: \"{}\" logged in successful",
                                           user_name_value->c_str());
                        valid_user = true;
                        login_status.token_ = *token;
                    }
                    else
                    {
                        valid_user = false;
                    }
                }
                login_status.user_name_ = user_name_value->c_str();
                login_status.success_ = valid_user;
            }
            else
            {
                SPDLOG_LOGGER_WARN(spdlog::get("multi_logger"),
                                   "An error occoured while logging in: wrong JSON");
                SPDLOG_LOGGER_DEBUG(spdlog::get("multi_logger"), "JSON to log in was: {}",
                                    body.c_str());
            }
            extra_data = std::move(login_status);
            co_return pro::make_proxy<core::http::proxy::Request>(std::move(new_parser.get()));
        }
    } // namespace request

    namespace response
    {
        NETDISK_CONTROLLER_RESPONSE(login)
        {
            const auto& login_status = std::any_cast<internal::LoginStatus&>(extra_data);
            boost::json::object json_value;
            if (login_status.success_)
            {
                json_value = {
                    {"loginStatus",           "success"},
                    {      "Token", login_status.token_}
                };
            }
            else
            {
                json_value = {
                    {"loginStatus", "fail"}
                };
            }
            const auto json_string = boost::json::serialize(json_value);
            co_return co_await connection.staticBodyReply(boost::beast::http::status::ok,
                                                          json_string, json_string.size(),
                                                          "application/json", config);
        }
    } // namespace response
} // namespace netdisk::controller::http::security
