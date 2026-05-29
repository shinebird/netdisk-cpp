#pragma once

#include "netdisk-cpp/controller/http/security/AuthorizationManager.hpp"

namespace netdisk::controller::http::security::authorization_rules
{
    auto serviceGet(const repository::User& user, const boost::urls::matches& match) -> bool;
    auto servicePost(const repository::User& user, const boost::urls::matches& match) -> bool;
}
