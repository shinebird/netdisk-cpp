#pragma once

#include "netdisk-cpp/repository/generic/DataBaseConnection.hpp"
#include "netdisk-cpp/repository/generic/User.hpp"

namespace netdisk::repository
{
    template <typename Impl, typename RowImpl>
        requires(std::same_as<typename Impl::RowImpl, RowImpl>)
    auto queryUserById(std::int64_t id, database::Connection<Impl, RowImpl>* connection,
                       std::string_view custom_query = {}) -> User
    {
        static constexpr std::string_view default_query =
            "SELECT id, username, password, auth FROM users WHERE id = ?";
        User user;
        connection->query(
            custom_query.empty() ? default_query : custom_query,
            [&user](database::Row<Impl::RowImpl>& row) -> void
            {
                user.setId(row.template getColumn<int>(0));
                user.setUsername(row.template getColumn<std::string>(1));
                user.setPassword(row.template getColumn<std::string>(2));
                user.setAuth(User::Auth(row.template getColumn<int>(3)));
            },
            id);
        return user;
    }

    template <typename Impl, typename RowImpl>
        requires(std::same_as<typename Impl::RowImpl, RowImpl>)
    auto queryUserByUsername(std::string_view username,
                             database::Connection<Impl, RowImpl>* connection,
                             std::string_view custom_query = {}) -> User
    {
        static constexpr std::string_view default_query =
            "SELECT id, username, password, auth FROM users WHERE username = ?";
        User user;
        connection->query(
            custom_query.empty() ? default_query : custom_query,
            [&user](database::Row<Impl::RowImpl>& row) -> void
            {
                user.setId(row.template getColumn<int>(0));
                user.setUsername(row.template getColumn<std::string>(1));
                user.setPassword(row.template getColumn<std::string>(2));
                user.setAuth(User::Auth(row.template getColumn<int>(3)));
            },
            username.data());
        return user;
    }
} // namespace netdisk::repository
