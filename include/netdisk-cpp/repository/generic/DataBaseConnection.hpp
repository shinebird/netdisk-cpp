#pragma once

#include <functional>
#include <string_view>

namespace netdisk::repository::database
{
    template <typename Impl> class Row
    {
        public:
            template <typename T> auto getColumn(std::size_t index) -> T
            {
                return static_cast<Impl*>(this)->template getColumn<T>(index);
            }
    };

    template <typename Impl, typename RowImpl> class Connection
    {

        public:
            using QueryHandler = std::function<void(Row<RowImpl>&)>;
            auto connect() -> void;
            template <bool CopyArgs = true, typename... Args>
            auto exec(std::string_view command, Args... args) -> void
            {
                return static_cast<Impl*>(this)->template exec<CopyArgs, Args...>(command, args...);
            }
            template <bool CopyArgs = true, typename... Args>
            auto query(std::string_view command, QueryHandler&& handler, Args... args) -> void
            {
                return static_cast<Impl*>(this)->template query<CopyArgs, Args...>(
                    command, std::move(handler), args...);
            }
    };
} // namespace netdisk::repository::database
