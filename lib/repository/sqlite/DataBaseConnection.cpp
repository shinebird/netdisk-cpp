#include "netdisk-cpp/repository/sqlite/DataBaseConnection.hpp"

namespace netdisk::repository::database::sqlite
{
    Row::Row(SQLite::Statement* query) : query_(query) {}

    template <> auto Row::getColumn(std::size_t index) -> int 
    {
        return query_->getColumn(index).getInt();
    }

    template <> auto Row::getColumn(std::size_t index) -> double 
    {
        return query_->getColumn(index).getDouble();
    }

    template <> auto Row::getColumn(std::size_t index) -> std::string 
    {
        return query_->getColumn(index).getString();
    }

    template <> auto Row::getColumn(std::size_t index) -> std::pair<const void*, std::size_t> 
    {
        const auto column = query_->getColumn(index);
        return {column.getBlob(), static_cast<std::size_t>(column.getBytes())};
    }

    Connection::Connection(std::string_view db_path) : database_(db_path, SQLite::OPEN_READWRITE) {}

    auto Connection::connect() -> void
    {
        // Database is connected while creating SQLite::Database instance
    }
} // namespace netdisk::repository::database::sqlite
