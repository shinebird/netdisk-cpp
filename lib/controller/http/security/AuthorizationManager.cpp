#include "netdisk-cpp/controller/http/security/AuthorizationManager.hpp"

namespace netdisk::controller::http::security
{
    AuthorizationManager::AuthorizationManager()
    {
        for (auto& item : authorization_routers_)
        {
            item = std::make_unique<boost::urls::router<AuthorizationHandler>>();
        }
    }

    auto AuthorizationManager::addAuthorizationHandler(boost::beast::http::verb method,
                                                       std::string_view pattern,
                                                       AuthorizationHandler&& handler) -> void
    {
        this->authorization_routers_[std::to_underlying(method)]->insert(pattern,
                                                                         std::move(handler));
    }

    [[nodiscard]] auto AuthorizationManager::authorize(boost::beast::http::verb method,
                                                       std::string_view target,
                                                       const repository::User& user) const -> bool
    {
        boost::urls::matches url_match;
        if (const auto* handler = authorization_routers_[std::to_underlying(method)]->find(
                boost::core::string_view(target), url_match))
        {
            return (*handler)(user, url_match);
        }
        // 默认放行
        return true;
    }
} // namespace netdisk::controller::http::security
