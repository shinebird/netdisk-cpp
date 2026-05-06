#pragma once

#include <boost/describe.hpp>

#include <netdisk-cpp/utils/concept/Common.hpp>

namespace netdisk::utils::reflect
{
    template <Enum T> auto getEnumName(T t) -> std::string
    {
        bool found = false;
        std::string ret;
        boost::mp11::mp_for_each<boost::describe::describe_enumerators<std::remove_reference_t<T>>>(
            [&](auto&& D)
            {
                if (!found && D.value == t)
                {
                    found = true;
                    ret = D.name;
                }
            });
        return ret;
    }

    template <Enum T> auto getEnumValue(const std::string& t) -> T
    {
        bool found = false;
        T ret;
        boost::mp11::mp_for_each<boost::describe::describe_enumerators<std::remove_reference_t<T>>>(
            [&](auto&& D)
            {
                if (!found && D.name == t)
                {
                    found = true;
                    ret = D.value;
                }
            });
        return ret;
    }
}