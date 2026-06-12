#include "netdisk-cpp/controller/generic/security/UserAuthenticator.hpp"

namespace netdisk::controller::security
{
    void UserAuthenticator::loginUser(const repository::User& user, const std::string& token)
    {
        MapType::accessor acc;
        map_.insert(acc, user.getUsername());
        login_users_.insert({user.getUsername(), user});
        acc->second.add(token);
    }

    auto UserAuthenticator::hasToken(const std::string& username, const std::string& token) -> bool
    {
        MapType::accessor acc;
        if (!map_.find(acc, username))
        {
            return false;
        }
        return acc->second.validateAndRefresh(token);
    }

    auto UserAuthenticator::getTokens(const std::string& username) const
        -> std::optional<std::vector<std::string>>
    {
        MapType::const_accessor cacc;
        if (!map_.find(cacc, username))
        {
            return std::nullopt;
        }
        return cacc->second.getAll();
    }

    auto UserAuthenticator::removeToken(const std::string& username, const std::string& token)
        -> bool
    {
        MapType::accessor acc;
        if (!map_.find(acc, username))
        {
            return false;
        }
        return acc->second.remove(token);
    }

    auto UserAuthenticator::getUser(const std::string& username) const
        -> std::optional<const repository::User&>
    {
        auto iter = login_users_.find(username);
        if (iter != login_users_.end())
        {
            return iter->second;
        }
        return std::nullopt;
    }

    void TokenList::removeNode(TokenNode* node)
    {
        index_.erase(node->token_);
        lru_list_.erase(lru_list_.iterator_to(*node));
        delete node;
    }

    void TokenList::promoteToFront(TokenNode* node)
    {
        lru_list_.splice(lru_list_.begin(), lru_list_, lru_list_.iterator_to(*node));
    }

    TokenList::~TokenList() { clear(); }

    void TokenList::add(std::string token)
    {
        if (auto iter = index_.find(token); iter != index_.end())
        {
            // 已存在 → LRU 刷新
            promoteToFront(iter->second);
            return;
        }

        // 新建节点
        auto* node = new TokenNode(std::move(token));
        lru_list_.push_front(*node);
        index_.try_emplace(node->token_, node);

        // 超限淘汰
        while (lru_list_.size() > MAX_TOKENS_)
        {
            TokenNode& oldest = lru_list_.back();
            removeNode(&oldest);
        }
    }

    auto TokenList::validateAndRefresh(const std::string& token) -> bool
    {
        auto iter = index_.find(token);
        if (iter == index_.end())
        {
            return false;
        }

        promoteToFront(iter->second);
        return true;
    }

    auto TokenList::getAll() const -> std::vector<std::string>
    {
        std::vector<std::string> result;
        result.reserve(lru_list_.size());
        for (const auto& node : lru_list_)
        {
            result.push_back(node.token_);
        }
        return result;
    }

    auto TokenList::remove(const std::string& token) -> bool
    {
        auto iter = index_.find(token);
        if (iter == index_.end())
        {
            return false;
        }
        removeNode(iter->second);
        return true;
    }

    auto TokenList::empty() const -> bool { return lru_list_.empty(); }

    void TokenList::clear()
    {
        index_.clear();
        lru_list_.clear_and_dispose([](TokenNode* p) { delete p; });
    }
} // namespace netdisk::controller::security
