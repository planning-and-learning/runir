#ifndef RUNIR_FORMATTER_HPP_
#define RUNIR_FORMATTER_HPP_

#include "runir/config.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <yggdrasil/formatting/formatter.hpp>

namespace runir
{

template<typename T>
std::string to_string(const T& element)
{
    return fmt::format("{}", element);
}

namespace formatting
{

inline std::string joined_components(std::string_view keyword, const std::vector<std::string>& components)
{
    if (components.empty())
        return std::string(keyword);
    return fmt::format("{} {}", keyword, fmt::join(components, " "));
}

template<typename... Components>
std::string components(std::string_view keyword, Components&&... args)
{
    return joined_components(keyword, std::vector<std::string> { fmt::format("{}", std::forward<Components>(args))... });
}

template<typename Objects>
std::vector<std::string> object_names(Objects objects)
{
    auto result = std::vector<std::string> {};
    for (auto object : objects)
        result.push_back(fmt::format("{:?}", std::string(object.get_name().str())));
    return result;
}

template<typename Objects>
std::string components_with_objects(std::string_view keyword, Objects objects)
{
    return joined_components(keyword, object_names(objects));
}

template<typename Head, typename Objects>
std::string components_with_objects(std::string_view keyword, Head&& head, Objects objects)
{
    auto components = std::vector<std::string> { fmt::format("{}", std::forward<Head>(head)) };
    auto names = object_names(objects);
    components.insert(components.end(), names.begin(), names.end());
    return joined_components(keyword, components);
}

}  // namespace formatting

}  // namespace runir

#endif
