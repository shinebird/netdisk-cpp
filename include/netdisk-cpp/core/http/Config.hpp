#pragma once

#include "netdisk-cpp/core/http/config/CORS.hpp"

#include <cstdint>

namespace netdisk
{
    namespace controller
    {
        namespace security
        {
            class UserAuthenticator;
        }
        namespace http::security
        {
            class AuthorizationManager;
        }
    } // namespace controller
    namespace repository::database
    {
#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE
        namespace sqlite
        {
            class Connection;
        }
#endif
    } // namespace repository::database

    namespace core::http
    {
        class Config
        {
            public:
                Config(std::uint16_t port, std::uint64_t num_threads,
#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE
                       repository::database::sqlite::Connection* database_connection,
#endif
                       controller::security::UserAuthenticator* user_authenticator,
                       controller::http::security::AuthorizationManager* authorization_manager);
                [[nodiscard]] auto getCORS() -> config::CORS&;
                [[nodiscard]] auto getPort() const -> std::uint16_t;
                [[nodiscard]] auto getNumThreads() const -> std::uint64_t;
                [[nodiscard]] auto getDatabaseConnection() const ->
#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE
                    repository::database::sqlite::Connection*;
#endif
                [[nodiscard]] auto getUserAuthenticator() const
                    -> controller::security::UserAuthenticator*;
                [[nodiscard]] auto getAuthorizationManager() const
                    -> controller::http::security::AuthorizationManager*;

            private:
                std::uint16_t port_;
                std::uint64_t num_threads_;
                config::CORS CORS_;
#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE
                repository::database::sqlite::Connection* database_connection_;
#endif
                controller::security::UserAuthenticator* user_authenticator_;
                controller::http::security::AuthorizationManager* authorization_manager_;
        };
    } // namespace core::http

} // namespace netdisk
