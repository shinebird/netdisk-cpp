#pragma once

#include <concepts>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>


#include <boost/beast.hpp>

#include "netdisk-cpp/core/http/Response.hpp"
#include "netdisk-cpp/utils/url/Router.hpp"

namespace netdisk::core::http::config
{
    using CORSHandler = std::function<std::size_t()>;

    class CORSRegistration
    {
        public:
            auto setAllowOrigin(std::string_view allow_origin) -> void;
            template <typename... Verbs>
                requires(std::same_as<Verbs, boost::beast::http::verb>, ...)
            auto setAllowMethods(Verbs... allow_method) -> void
            {
                (allow_methods_.emplace_back(allow_method), ...);
            }
            template <typename... Fields>
                requires(std::same_as<Fields, boost::beast::http::field>, ...)
            auto setAllowHeaders(Fields... allow_header) -> void
            {
                (allow_headers_.emplace_back(allow_header), ...);
            }
            auto setMaxAge(std::uint64_t max_age) -> void;
            [[nodiscard]] auto getAllowOrigin() const -> std::string;
            [[nodiscard]] auto getAllowMethods() const -> std::string;
            [[nodiscard]] auto getAllowHeaders() const -> std::string;
            [[nodiscard]] auto getMaxAge() const -> std::string;

        private:
            std::string allow_origin_;
            std::vector<boost::beast::http::verb> allow_methods_;
            std::vector<boost::beast::http::field> allow_headers_;
            std::uint64_t max_age_;
    };

    class CORS
    {
        public:
            auto addMapping(std::string_view pattern, CORSRegistration registration) -> void;
            auto addHeaders(boost::core::string_view target, ResponseView response_view, std::error_code& error_code) -> void;

        private:
            std::vector<CORSRegistration> registrations_;
            boost::urls::router<CORSHandler> cors_router_;
    };
} // namespace netdisk::core::http::config
