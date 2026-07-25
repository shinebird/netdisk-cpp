#pragma once

namespace netdisk::utils
{
    template <class... Ts> struct Overloads : Ts...
    {
            using Ts::operator()...;
    };
} // namespace netdisk::utils
