#include "netdisk-cpp/controller/http/security/AuthorizationRules.hpp"

#include <utility>

namespace netdisk::controller::http::security::authorization_rules
{
    auto serviceGet(const repository::User& user, const boost::urls::matches& match) -> bool
    {
        return std::to_underlying(user.getAuth()) >= std::to_underlying(data::User::Auth::normal);
    }

    auto servicePost(const repository::User& user, const boost::urls::matches& match) -> bool
    {
        return std::to_underlying(user.getAuth()) >= std::to_underlying(data::User::Auth::normal);
    }
} // namespace netdisk::controller::http::security::authorization_rules
