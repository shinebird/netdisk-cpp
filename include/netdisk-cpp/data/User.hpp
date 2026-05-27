#pragma once

#include <cstdint>
#include <limits>
#include <string>

namespace netdisk::data
{
    class User
    {
        public:
            enum class Auth : unsigned char
            {
                normal = 0,
                administrator = 1,
                max_ = 1,
            };

            static constexpr auto invalid_id_ = std::numeric_limits<std::int64_t>::max();

        protected:
            User() = default;
            std::int64_t id_ = invalid_id_;
            std::string username_;
            std::string password_;
            Auth auth_;
    };
} // namespace netdisk::data
