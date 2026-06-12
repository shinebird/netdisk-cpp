#include "netdisk-cpp/controller/http/security/VerifyUserToken.hpp"
#include "netdisk-cpp/controller/generic/security/UserAuthenticator.hpp"
#include "netdisk-cpp/controller/http/security/AuthorizationManager.hpp"
#include "netdisk-cpp/utils/jwt/JWT.hpp"

#include <boost/url.hpp>

namespace netdisk::controller::http::security
{
    auto verifyUserToken(core::http::RequestView& request, core::http::Config& config) -> bool
    {
        auto target = request->target();
        repository::User user;
        if (config.getAuthorizationManager()->authorize(request->method(), target, user))
        {
            return true;
        }
        auto user_name_header = request->operator[]("username");
        auto token_header = request->operator[]("Token");
        boost::urls::url_view url = target;
        auto user_name_url = url.params().get_or("username");
        auto token_url = url.params().get_or("token");
        auto user_name =
            !user_name_header.empty() ? user_name_header : boost::core::string_view(user_name_url);
        auto token = !token_header.empty() ? token_header : boost::core::string_view(token_url);
        if (config.getUserAuthenticator()->hasToken(user_name, token))
        {
            if (utils::jwt::verifyUserToken(user_name, token))
            {
                return config.getAuthorizationManager()->authorize(
                    request->method(), target, *config.getUserAuthenticator()->getUser(user_name));
            }
            config.getUserAuthenticator()->removeToken(user_name, token);
            return false;
        }
        return false;
    }
} // namespace netdisk::controller::http::security
