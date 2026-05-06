#include "netdisk-cpp/core/http/config/CORS.hpp"
#include "netdisk-cpp/utils/concept/Container.hpp"

#include <boost/beast/http/field.hpp>
#include <boost/core/detail/string_view.hpp>

#include <concepts>
#include <cstddef>
#include <ranges>
#include <stdexcept>
#include <system_error>

namespace netdisk::core::http::config
{
    namespace internal
    {
        template <netdisk::utils::reflect::ForwardContainer Container>
            requires(std::same_as<typename Container::value_type, boost::beast::http::field> ||
                     std::same_as<typename Container::value_type, boost::beast::http::verb>)
        static auto toString(const Container& items) -> std::string
        {
            std::size_t total_length = 0;
            total_length += 2 * std::max(items.size() - 1, 0ULL);
            std::vector<boost::core::string_view> strings;
            strings.reserve(items.size());
            for (const auto& method : items)
            {
                auto method_string = boost::beast::http::to_string(method);
                if (method_string.starts_with("<unknown"))
                {
                    method_string = "*";
                }
                strings.emplace_back(method_string);
                total_length += method_string.size();
            }
            std::string result;
            result.reserve(total_length);
            for (const auto& [index, str] : std::views::enumerate(strings) |
                                                std::views::take(std::max(items.size() - 1, 0ULL)))
            {
                result += str;
                result += ", ";
            }
            result += strings[items.size() - 1];
            return result;
        }
    } // namespace internal

    auto CORSRegistration::setAllowOrigin(std::string_view allow_origin) -> void
    {
        this->allow_origin_ = allow_origin;
    }

    auto CORSRegistration::setMaxAge(std::uint64_t max_age) -> void { this->max_age_ = max_age; }

    auto CORSRegistration::getAllowOrigin() const -> std::string { return this->allow_origin_; }

    auto CORSRegistration::getAllowMethods() const -> std::string
    {
        return internal::toString(allow_methods_);
    }

    auto CORSRegistration::getAllowHeaders() const -> std::string
    {
        return internal::toString(allow_headers_);
    }

    auto CORSRegistration::getMaxAge() const -> std::string { return std::format("{}", max_age_); }

    auto CORS::addMapping(std::string_view pattern, CORSRegistration registration) -> void
    {
        this->registrations_.emplace_back(std::move(registration));
        this->cors_router_.insert(pattern, [current_size = this->registrations_.size()]()
                                  { return current_size - 1; });
    }

    auto CORS::addHeaders(boost::core::string_view target, ResponseView response_view,
                          std::error_code& error_code) -> void
    {
        boost::urls::matches url_match;
        if (const auto* handler = cors_router_.find(target, url_match))
        {
            const auto index = (*handler)();
            const auto current_registration = registrations_.at(index);
            boost::beast::http::fields current_fields;
            response_view->swap(current_fields);
            current_fields.set(boost::beast::http::field::access_control_allow_origin,
                               current_registration.getAllowOrigin());
            current_fields.set(boost::beast::http::field::access_control_allow_methods,
                               current_registration.getAllowMethods());
            current_fields.set(boost::beast::http::field::access_control_allow_headers,
                               current_registration.getAllowHeaders());
            current_fields.set(boost::beast::http::field::access_control_max_age,
                               current_registration.getMaxAge());
            response_view->swap(current_fields);
            error_code.clear();
        }
        else
        {
            error_code = std::make_error_code(std::errc::invalid_argument);
        }
    }
} // namespace netdisk::core::http::config
