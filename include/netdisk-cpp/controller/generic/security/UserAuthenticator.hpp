#pragma once

#include <oneapi/tbb/concurrent_unordered_map.h>
#include <oneapi/tbb/concurrent_unordered_set.h>

#include <optional>
#include <string>
#include <string_view>

#include "netdisk-cpp/repository/generic/User.hpp"

namespace netdisk::controller::security
{
    class UserAuthenticator
    {
        public:
            auto loginUser(const repository::User& user, std::string_view token) -> bool;
            auto getUserToken(std::string_view user_name) const -> std::optional<const std::pair<
                repository::User, oneapi::tbb::concurrent_unordered_set<std::string>>&>;

        private:
            oneapi::tbb::concurrent_unordered_map<
                std::string,
                std::pair<repository::User, oneapi::tbb::concurrent_unordered_set<std::string>>>
                login_user_tokens_;
    };
} // namespace netdisk::controller::security
