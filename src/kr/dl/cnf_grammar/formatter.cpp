#include "runir/kr/dl/cnf_grammar/formatter.hpp"

#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/cnf_grammar/derivation_rule_view.hpp"
#include "runir/kr/dl/cnf_grammar/grammar_view.hpp"
#include "runir/kr/dl/cnf_grammar/non_terminal_view.hpp"
#include "runir/kr/dl/cnf_grammar/substitution_rule_view.hpp"

#include <fmt/format.h>
#include <string_view>
#include <utility>
#include <variant>
#include <yggdrasil/formatting/cista_formatters.hpp>

namespace dl = runir::kr::dl;
namespace dc = runir::kr::dl::cnf_grammar;

template<dl::CategoryTag Category>
fmt::format_context::iterator fmt::formatter<urd::index_view<dc::NonTerminal<Category>, dc::ConstructorRepository>>::format(
    const urd::index_view<dc::NonTerminal<Category>, dc::ConstructorRepository>& value,
    fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value.name().str()); }

template<dl::CategoryTag Category>
fmt::format_context::iterator fmt::formatter<urd::index_view<dc::DerivationRule<Category>, dc::ConstructorRepository>>::format(
    const urd::index_view<dc::DerivationRule<Category>, dc::ConstructorRepository>& value,
    fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "({} (", value.lhs());
    std::visit([&out](auto constructor) { out = fmt::format_to(out, "{}", std::pair { constructor, dl::Unparenthesized {} }); }, value.rhs());
    return fmt::format_to(out, "))");
}

template<dl::CategoryTag Category>
fmt::format_context::iterator fmt::formatter<urd::index_view<dc::SubstitutionRule<Category>, dc::ConstructorRepository>>::format(
    const urd::index_view<dc::SubstitutionRule<Category>, dc::ConstructorRepository>& value,
    fmt::format_context& context) const
{ return fmt::format_to(context.out(), "({} ({}))", value.lhs(), value.rhs()); }

fmt::format_context::iterator
fmt::formatter<urd::index_view<dc::Grammar, dc::ConstructorRepository>>::format(const urd::index_view<dc::Grammar, dc::ConstructorRepository>& value,
                                                                                fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(\n");
    const auto write = [&out](auto rules)
    {
        for (auto rule : rules)
            out = fmt::format_to(out, "    {}\n", rule);
    };
    write(value.derivation_rules<dl::ConceptTag>());
    write(value.derivation_rules<dl::RoleTag>());
    write(value.derivation_rules<dl::BooleanTag>());
    write(value.derivation_rules<dl::NumericalTag>());
    write(value.substitution_rules<dl::ConceptTag>());
    write(value.substitution_rules<dl::RoleTag>());
    write(value.substitution_rules<dl::BooleanTag>());
    write(value.substitution_rules<dl::NumericalTag>());
    return fmt::format_to(out, ")\n");
}

// Each category (dl::CategoryTags).
template struct fmt::formatter<urd::index_view<dc::NonTerminal<dl::ConceptTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dc::NonTerminal<dl::RoleTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dc::NonTerminal<dl::BooleanTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dc::NonTerminal<dl::NumericalTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dc::DerivationRule<dl::ConceptTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dc::DerivationRule<dl::RoleTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dc::DerivationRule<dl::BooleanTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dc::DerivationRule<dl::NumericalTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dc::SubstitutionRule<dl::ConceptTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dc::SubstitutionRule<dl::RoleTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dc::SubstitutionRule<dl::BooleanTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dc::SubstitutionRule<dl::NumericalTag>, dc::ConstructorRepository>>;
