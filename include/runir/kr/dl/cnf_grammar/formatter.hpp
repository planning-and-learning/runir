#ifndef RUNIR_CNF_GRAMMAR_FORMATTER_HPP_
#define RUNIR_CNF_GRAMMAR_FORMATTER_HPP_

#include "runir/kr/dl/cnf_grammar/derivation_rule_view.hpp"
#include "runir/kr/dl/cnf_grammar/grammar_view.hpp"
#include "runir/kr/dl/cnf_grammar/non_terminal_view.hpp"
#include "runir/kr/dl/cnf_grammar/substitution_rule_view.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/formatter.hpp"

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

namespace runir::kr::dl::cnf_grammar
{

template<typename... Views>
std::string constructor_body(const std::variant<Views...>& view)
{
    return std::visit([](auto arg) { return runir::kr::dl::record_body(arg); }, view);
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string rule(urd::index_view<DerivationRule<Category>, C> view)
{ return fmt::format("({} ({}))", view.get_lhs(), constructor_body(view.get_rhs())); }

template<runir::kr::dl::CategoryTag Category, typename C>
std::string rule(urd::index_view<SubstitutionRule<Category>, C> view)
{ return fmt::format("({} ({}))", view.get_lhs(), view.get_rhs()); }

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
    append_rules(text, view.template get_substitution_rules<runir::kr::dl::ConceptTag>());
    append_rules(text, view.template get_substitution_rules<runir::kr::dl::RoleTag>());
    append_rules(text, view.template get_substitution_rules<runir::kr::dl::BooleanTag>());
    append_rules(text, view.template get_substitution_rules<runir::kr::dl::NumericalTag>());
    text += ")\n";
    return text;
}

}  // namespace runir::kr::dl::cnf_grammar

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::cnf_grammar::NonTerminal<Category>, C>> : fmt::formatter<std::string_view>
{
    using View = urd::index_view<runir::kr::dl::cnf_grammar::NonTerminal<Category>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(view.get_name().str(), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::cnf_grammar::DerivationRule<Category>, C>> : fmt::formatter<std::string_view>
{
    using View = urd::index_view<runir::kr::dl::cnf_grammar::DerivationRule<Category>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::rule(view), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::cnf_grammar::SubstitutionRule<Category>, C>> : fmt::formatter<std::string_view>
{
    using View = urd::index_view<runir::kr::dl::cnf_grammar::SubstitutionRule<Category>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::rule(view), ctx); }
};

template<typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::cnf_grammar::Grammar, C>> : fmt::formatter<std::string_view>
{
    using View = urd::index_view<runir::kr::dl::cnf_grammar::Grammar, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::grammar(view), ctx); }
};

#endif
