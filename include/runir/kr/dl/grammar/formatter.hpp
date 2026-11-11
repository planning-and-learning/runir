#ifndef RUNIR_GRAMMAR_FORMATTER_HPP_
#define RUNIR_GRAMMAR_FORMATTER_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/formatter.hpp"
#include "runir/kr/dl/grammar/derivation_rule_view.hpp"
#include "runir/kr/dl/grammar/grammar_view.hpp"
#include "runir/kr/dl/grammar/non_terminal_view.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <string_view>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/formatting/cista_formatters.hpp>
#include <yggdrasil/formatting/formatter.hpp>

namespace runir::kr::dl::grammar
{

template<typename... Views>
std::string constructor_body(const std::variant<Views...>& view)
{
    return std::visit([](auto arg) { return runir::kr::dl::record_body(arg); }, view);
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string rule(urd::index_view<DerivationRule<Category>, C> view)
{
    auto rhs = urd::Store<urd::sequence<std::string>> {};
    for (auto symbol : view.get_rhs())
    {
        rhs.push_back(std::visit(
            []<typename Arg>(Arg arg)
            {
                if constexpr (std::same_as<Arg, urd::index_view<NonTerminal<Category>, C>>)
                    return fmt::format("{}", arg);
                else
                    return constructor_body(arg);
            },
            symbol));
    }

    return fmt::format("({} ({}))", view.get_lhs(), fmt::join(rhs, " or "));
}

template<typename Rules>
void append_rules(std::string& text, Rules rules)
{
    for (auto rule : rules)
        text += fmt::format("    {}\n", rule);
}

template<typename View>
std::string grammar(View view)
{
    auto text = std::string("(\n");
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::ConceptTag>());
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::RoleTag>());
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::BooleanTag>());
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::NumericalTag>());
    text += ")\n";
    return text;
}

}  // namespace runir::kr::dl::grammar

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::grammar::NonTerminal<Category>, C>> : fmt::formatter<std::string_view>
{
    using View = urd::index_view<runir::kr::dl::grammar::NonTerminal<Category>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(view.get_name().str(), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::grammar::DerivationRule<Category>, C>> : fmt::formatter<std::string_view>
{
    using View = urd::index_view<runir::kr::dl::grammar::DerivationRule<Category>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::grammar::rule(view), ctx); }
};

template<typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::grammar::GrammarTag, C>> : fmt::formatter<std::string_view>
{
    using View = urd::index_view<runir::kr::dl::grammar::GrammarTag, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::grammar::grammar(view), ctx); }
};

#endif
