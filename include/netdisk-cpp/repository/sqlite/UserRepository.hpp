#pragma once

#include "netdisk-cpp/repository/generic/User.hpp"

namespace netdisk::repository
{
    namespace database::sqlite
    {
        class Connection;
    }

    namespace sqlite
    {
        auto queryUserById(std::int64_t id, database::sqlite::Connection* connection) -> User;
        auto queryUserByUsername(std::string_view username,
                                 database::sqlite::Connection* connection) -> User;
    } // namespace sqlite
} // namespace netdisk::repository
