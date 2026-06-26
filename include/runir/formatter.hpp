#ifndef RUNIR_FORMATTER_HPP_
#define RUNIR_FORMATTER_HPP_

#include "runir/config.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <yggdrasil/formatting/formatter.hpp>

namespace runir
{


struct SExpressionNode
{
    bool list = false;
    std::string token;
    std::vector<SExpressionNode> children;
};

inline void skip_sexpression_whitespace(std::string_view text, std::size_t& pos)
{
    while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos])))
        ++pos;
}

inline std::optional<SExpressionNode> parse_sexpression(std::string_view text, std::size_t& pos)
{
    skip_sexpression_whitespace(text, pos);
    if (pos >= text.size())
        return std::nullopt;

    if (text[pos] == '(')
    {
        ++pos;
        auto node = SExpressionNode {};
        node.list = true;
        while (true)
        {
            skip_sexpression_whitespace(text, pos);
            if (pos >= text.size())
                return std::nullopt;
            if (text[pos] == ')')
            {
                ++pos;
                return node;
            }
            auto child = parse_sexpression(text, pos);
            if (!child)
                return std::nullopt;
            node.children.push_back(std::move(*child));
        }
    }

    auto node = SExpressionNode {};
    if (text[pos] == '"')
    {
        const auto start = pos++;
        auto escaped = false;
        while (pos < text.size())
        {
            const auto ch = text[pos++];
            if (escaped)
                escaped = false;
            else if (ch == '\\')
                escaped = true;
            else if (ch == '"')
            {
                node.token = std::string(text.substr(start, pos - start));
                return node;
            }
        }
        return std::nullopt;
    }

    const auto start = pos;
    while (pos < text.size() && !std::isspace(static_cast<unsigned char>(text[pos])) && text[pos] != '(' && text[pos] != ')')
        ++pos;
    if (start == pos)
        return std::nullopt;
    node.token = std::string(text.substr(start, pos - start));
    return node;
}

inline bool has_nested_sexpression(const SExpressionNode& node)
{
    if (!node.list)
        return false;
    for (const auto& child : node.children)
        if (child.list)
            return true;
    return false;
}

inline std::string render_sexpression(const SExpressionNode& node, std::size_t indent = 0)
{
    if (!node.list)
        return node.token;
    if (node.children.empty())
        return "()";

    if (!has_nested_sexpression(node))
    {
        auto result = std::string("(");
        for (std::size_t i = 0; i < node.children.size(); ++i)
        {
            if (i > 0)
                result += ' ';
            result += render_sexpression(node.children[i], 0);
        }
        result += ')';
        return result;
    }

    auto result = std::string("(") + render_sexpression(node.children.front(), 0);
    for (std::size_t i = 1; i < node.children.size(); ++i)
        result += "\n" + std::string(indent + 4, ' ') + render_sexpression(node.children[i], indent + 4);
    result += "\n" + std::string(indent, ' ') + ")";
    return result;
}

inline std::string pretty_sexpression(std::string_view text)
{
    auto pos = std::size_t(0);
    auto node = parse_sexpression(text, pos);
    if (!node)
        return std::string(text);
    skip_sexpression_whitespace(text, pos);
    if (pos != text.size() || !node->list || !has_nested_sexpression(*node))
        return std::string(text);
    return render_sexpression(*node);
}

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
