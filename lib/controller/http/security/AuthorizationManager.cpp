#include "netdisk-cpp/controller/http/security/AuthorizationManager.hpp"

#include <boost/url.hpp>

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
        boost::urls::url url = *boost::urls::parse_relative_ref(target);
        if (const auto* handler = authorization_routers_[std::to_underlying(method)]->find(
                url.remove_query().encoded_segments(), url_match))
        {
            return user.getId() != data::User::invalid_id_ && (*handler)(user, url_match);
        }
        // 默认放行
        return true;
    }
} // namespace netdisk::controller::http::security
