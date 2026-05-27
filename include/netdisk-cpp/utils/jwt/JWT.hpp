#pragma once

#include <string>
#include <string_view>
#include <optional>

#include <jwt-cpp/traits/boost-json/defaults.h>

#include <jwt-cpp/jwt.h>

namespace netdisk::utils::jwt
{
    auto createUserToken(std::string_view user_name) -> std::optional<std::string>;
    auto verifyUserToken(std::string_view user_name, std::string_view token) -> bool;
} // namespace netdisk::utils::jwt
