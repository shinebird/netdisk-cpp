#include "netdisk-cpp/core/http/Config.hpp"

namespace netdisk::core::http
{
    Config::Config(std::uint16_t port, std::uint64_t num_threads,
#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE
                   repository::database::sqlite::Connection* database_connection,
#endif
                   controller::security::UserAuthenticator* user_authenticator,
                   controller::http::security::AuthorizationManager* authorization_manager)
        : port_(port), num_threads_(num_threads), database_connection_(database_connection),
          user_authenticator_(user_authenticator), authorization_manager_(authorization_manager)
    {
    }

    auto Config::getCORS() -> config::CORS& { return this->CORS_; }

    auto Config::getPort() const -> std::uint16_t { return this->port_; }

    auto Config::getNumThreads() const -> std::uint64_t { return this->num_threads_; }

    auto Config::getDatabaseConnection() const ->
#ifdef NETDISK_REPOSITORY_DATABASE_SQLITE
        repository::database::sqlite::Connection*
#endif

    {
        return this->database_connection_;
    }

    auto Config::getUserAuthenticator() const -> controller::security::UserAuthenticator*
    {
        return this->user_authenticator_;
    }

    auto Config::getAuthorizationManager() const
        -> controller::http::security::AuthorizationManager*
    {
        return this->authorization_manager_;
    }
} // namespace netdisk::core::http
