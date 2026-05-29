#include "netdisk-cpp/controller/generic/security/UserAuthenticator.hpp"

#include <optional>

namespace netdisk::controller::security
{
    auto UserAuthenticator::loginUser(const repository::User& user, std::string_view token) -> bool
    {
        auto iter = login_user_tokens_.find(user.getUsername());
        if (iter != login_user_tokens_.end())
        {
            iter->second.first = user;
            const auto& [_, result] = iter->second.second.emplace(std::string(token));
            return result;
        }
        const auto& [_, result] = login_user_tokens_.insert({
            user.getUsername(), {user, {std::string(token)}}
        });
        return result;
    }

    auto UserAuthenticator::getUserToken(std::string_view user_name) const -> std::optional<
        const std::pair<repository::User, oneapi::tbb::concurrent_unordered_set<std::string>>&>
    {
        auto iter = login_user_tokens_.find(std::string(user_name));
        if (iter != login_user_tokens_.end())
        {
            return iter->second;
        }
        return std::nullopt;
    }
} // namespace netdisk::controller::security
