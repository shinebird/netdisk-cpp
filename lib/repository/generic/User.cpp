#include "netdisk-cpp/repository/generic/User.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>
#include <utility>

namespace netdisk::repository
{
    auto User::getId() const -> std::int64_t { return this->id_; }

    auto User::getUsername() const -> std::string { return this->username_; }

    auto User::getPassword() const -> std::string { return this->password_; }

    auto User::getAuth() const -> Auth { return this->auth_; }

    void User::setId(std::int64_t id) { this->id_ = id; }

    void User::setUsername(std::string_view username) { this->username_ = username; }

    void User::setPassword(std::string_view password) { this->password_ = password; }

    void User::setAuth(Auth auth)
    {
        if (std::to_underlying(auth) > std::to_underlying(Auth::max_)) [[unlikely]]
        {
            SPDLOG_LOGGER_WARN(spdlog::get("multi_logger"), "Invalid auth: {}",
                               std::to_underlying(auth));
        }
        this->auth_ = auth;
    }
} // namespace netdisk::repository
