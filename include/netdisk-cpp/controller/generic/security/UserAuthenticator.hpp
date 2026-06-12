#pragma once

#include <oneapi/tbb/concurrent_hash_map.h>
#include <oneapi/tbb/concurrent_unordered_map.h>

#include <boost/intrusive/list.hpp>

#include "netdisk-cpp/repository/generic/User.hpp"
#include "netdisk-cpp/utils/string/StringUtils.hpp"

#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace bi = boost::intrusive;

namespace netdisk::controller::security
{

    // ============================================================
    // 侵入式节点
    // ============================================================
    struct TokenNode : public bi::list_base_hook<bi::link_mode<bi::safe_link>>
    {
            std::string token_;
            explicit TokenNode(std::string t) : token_(std::move(t)) {}
            TokenNode(const TokenNode&) = delete;
            auto operator=(const TokenNode&) -> TokenNode& = delete;
    };

    // ============================================================
    // TokenList：intrusive list + unordered_map 索引
    // ============================================================
    class TokenList
    {
            using ListType = bi::list<TokenNode, bi::constant_time_size<true>>;
            using IndexMap =
                std::unordered_map<std::string, TokenNode*, utils::string::Hash, std::equal_to<>>;

            ListType lru_list_; // front=最新, back=最旧
            IndexMap index_;    // token_string → node* (O(1) 查找)

            static constexpr size_t MAX_TOKENS_ = 10;

            // 从两端同时移除并释放节点
            void removeNode(TokenNode* node);

            // 将已有节点提升到最前面
            void promoteToFront(TokenNode* node);

        public:
            TokenList() = default;
            ~TokenList();
            TokenList(const TokenList&) = delete;
            auto operator=(const TokenList&) -> TokenList& = delete;

            /// 添加 token；若已存在则刷新；超限淘汰最旧
            void add(std::string token);

            /// 验证 token 并刷新 LRU 顺序
            auto validateAndRefresh(const std::string& token) -> bool;

            /// 获取所有 token（新→旧）
            [[nodiscard]] auto getAll() const -> std::vector<std::string>;

            /// 移除指定 token
            auto remove(const std::string& token) -> bool;

            [[nodiscard]] auto empty() const -> bool;

            void clear();
    };

    // ============================================================
    // UserTokenCache（外层接口不变）
    // ============================================================
    class UserAuthenticator
    {
        private:
            using MapType = oneapi::tbb::concurrent_hash_map<std::string, TokenList>;
            MapType map_;
            oneapi::tbb::concurrent_unordered_map<std::string, repository::User> login_users_;

        public:
            void loginUser(const repository::User& user, const std::string& token);

            auto hasToken(const std::string& username, const std::string& token) -> bool;

            auto getTokens(const std::string& username) const
                -> std::optional<std::vector<std::string>>;

            auto removeToken(const std::string& username, const std::string& token) -> bool;

            auto getUser(const std::string& username) const
                -> std::optional<const repository::User&>;
    };

} // namespace netdisk::controller::security
