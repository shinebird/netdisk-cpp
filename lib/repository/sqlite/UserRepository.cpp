#include "netdisk-cpp/repository/generic/UserRepository.hpp"
#include "netdisk-cpp/repository/sqlite/DataBaseConnection.hpp"
#include "netdisk-cpp/repository/sqlite/UserRepository.hpp"

namespace netdisk::repository::sqlite
{
    auto queryUserById(std::int64_t id, database::sqlite::Connection* connection) -> User
    {
        return ::netdisk::repository::queryUserById(id, connection);
    }

    auto queryUserByUsername(std::string_view username, database::sqlite::Connection* connection)
        -> User
    {
        return ::netdisk::repository::queryUserByUsername(username, connection);
    }
} // namespace netdisk::repository::sqlite
