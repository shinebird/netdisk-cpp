#include "netdisk-cpp/utils/jwt/JWT.hpp"

#include <chrono>
#include <optional>

#include <spdlog/spdlog.h>

namespace netdisk::utils::jwt
{
    auto createUserToken(std::string_view user_name) -> std::optional<std::string>
    {
        auto token_builder = ::jwt::create()
                                 .set_subject(std::string(user_name))
                                 .set_issued_now()
                                 .set_expires_in(std::chrono::hours(2));
        std::error_code error_code;
        auto token = token_builder.sign(::jwt::algorithm::hs512{"secret-key"}, error_code);

        if (!error_code)
        {
            return token;
        }

        SPDLOG_LOGGER_WARN(spdlog::get("multi_logger"),
                           "An error occourred while creating JWT token for user \"{}\": {}",
                           user_name, error_code.message());
        return std::nullopt;
    }

    auto verifyUserToken(std::string_view user_name, std::string_view token) -> bool
    {
        try
        {
            auto decoded_token = ::jwt::decode(std::string(token));
            auto verifier = ::jwt::verify().allow_algorithm(::jwt::algorithm::hs512{"secret-key"});
            std::error_code error_code;
            verifier.verify(decoded_token, error_code);
            if (!error_code)
            {
                return true;
            }

            SPDLOG_LOGGER_INFO(spdlog::get("multi_logger"), "Invalid JWT token for user \"{}\": {}",
                               user_name, error_code.message());
            return false;
        }
        catch (const std::exception& e)
        {
            SPDLOG_LOGGER_WARN(spdlog::get("multi_logger"),
                               "An error occourred while verifying JWT token for user \"{}\": {}",
                               user_name, e.what());
            return false;
        }
    }
} // namespace netdisk::utils::jwt
