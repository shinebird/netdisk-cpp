#pragma once

#include "netdisk-cpp/repository/generic/DataBaseConnection.hpp"

#include <string>
#include <string_view>

#include <SQLiteCpp/SQLiteCpp.h>

namespace netdisk::repository::database::sqlite
{
    class Row : public database::Row<Row>
    {
        public:
            explicit Row(SQLite::Statement* query);
            template <typename T> auto getColumn(std::size_t index) -> T
            {
                static_assert(false, "Not implemented");
            }

            template <> auto getColumn(std::size_t index) -> int;
            template <> auto getColumn(std::size_t index) -> double;
            template <> auto getColumn(std::size_t index) -> std::string;
            template <> auto getColumn(std::size_t index) -> std::pair<const void*, std::size_t>;

        private:
            SQLite::Statement* query_;
    };

    class Connection : public database::Connection<Connection, Row>
    {
        public:
            using RowImpl = Row;
            explicit Connection(std::string_view db_path);
            auto connect() -> void;
            template <bool CopyArgs = true, typename... Args>
            auto exec(std::string_view command, Args... args) -> void
            {
                SQLite::Statement statement(database_, command.data());
                [&]<auto... Indices>(std::index_sequence<Indices...>)
                {
                    (
                        [&]<size_t I>
                        {
                            if constexpr (CopyArgs)
                            {
                                statement.bind(static_cast<int>(I + 1), args...[I]);
                            }
                            else
                            {
                                statement.bindNoCopy(static_cast<int>(I + 1), args...[I]);
                            }
                        }.template operator()<Indices>(),
                        ...);
                }(std::make_index_sequence<sizeof...(Args)>{});
                statement.exec();
            }
            template <bool CopyArgs = true, typename... Args>
            auto query(std::string_view command, QueryHandler&& handler, Args... args) -> void
            {
                SQLite::Statement statement(database_, command.data());
                [&]<auto... Indices>(std::index_sequence<Indices...>)
                {
                    (
                        [&]<size_t I>
                        {
                            if constexpr (CopyArgs)
                            {
                                statement.bind(static_cast<int>(I + 1), args...[I]);
                            }
                            else
                            {
                                statement.bindNoCopy(static_cast<int>(I + 1), args...[I]);
                            }
                        }.template operator()<Indices>(),
                        ...);
                }(std::make_index_sequence<sizeof...(Args)>{});
                Connection::RowImpl row(std::addressof(statement));
                while (statement.executeStep())
                {
                    handler(row);
                }
            }

        private:
            SQLite::Database database_;
    };
} // namespace netdisk::repository::database::sqlite
