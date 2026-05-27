#pragma once

#include "netdisk-cpp/data/User.hpp"

namespace netdisk::repository
{
    class User : public data::User
    {
        public:
            [[nodiscard]] auto getId() const -> std::int64_t;
            [[nodiscard]] auto getUsername() const -> std::string;
            [[nodiscard]] auto getPassword() const -> std::string;
            [[nodiscard]] auto getAuth() const -> Auth;
            void setId(std::int64_t id);
            void setUsername(std::string_view username);
            void setPassword(std::string_view password);
            void setAuth(Auth auth);
    };
} // namespace netdisk::repository
