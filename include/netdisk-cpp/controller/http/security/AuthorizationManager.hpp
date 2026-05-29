#pragma once

#include <array>

#include <boost/beast/http/verb.hpp>

#include "netdisk-cpp/repository/generic/User.hpp"
#include "netdisk-cpp/utils/url/Matches.hpp"
#include "netdisk-cpp/utils/url/Router.hpp"

namespace netdisk::controller::http::security
{
    using AuthorizationHandler =
        std::function<bool(const repository::User&, const boost::urls::matches&)>;

    class AuthorizationManager
    {
        public:
            AuthorizationManager();
            auto addAuthorizationHandler(boost::beast::http::verb method, std::string_view pattern,
                                         AuthorizationHandler&& handler) -> void;
            [[nodiscard]] auto authorize(boost::beast::http::verb method, std::string_view target,
                                         const repository::User& user) const -> bool;

        private:
            std::array<std::unique_ptr<boost::urls::router<AuthorizationHandler>>,
                       std::to_underlying(boost::beast::http::verb::unlink) + 1>
                authorization_routers_;
    };
} // namespace netdisk::controller::http::security
