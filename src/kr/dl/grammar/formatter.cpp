#include "runir/kr/dl/grammar/formatter.hpp"

#include "runir/kr/dl/grammar/constructor_repository.hpp"
#include "runir/kr/dl/grammar/derivation_rule_view.hpp"
#include "runir/kr/dl/grammar/grammar_view.hpp"
#include "runir/kr/dl/grammar/non_terminal_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <string_view>
#include <utility>
#include <variant>
#include <yggdrasil/formatting/cista_formatters.hpp>

namespace dl = runir::kr::dl;
namespace dg = runir::kr::dl::grammar;

template<dl::CategoryTag Category>
fmt::format_context::iterator fmt::formatter<urd::index_view<dg::NonTerminal<Category>, dg::ConstructorRepository>>::format(
    const urd::index_view<dg::NonTerminal<Category>, dg::ConstructorRepository>& value,
    fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value.get_name().str()); }

template<dl::CategoryTag Category>
fmt::format_context::iterator fmt::formatter<urd::index_view<dg::DerivationRule<Category>, dg::ConstructorRepository>>::format(
    const urd::index_view<dg::DerivationRule<Category>, dg::ConstructorRepository>& value,
    fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "({} (", value.get_lhs());
    auto separator = std::string_view {};
    for (auto symbol : value.get_rhs())
    {
        out = fmt::format_to(out, "{}", separator);
        separator = " or ";
        std::visit(
            [&out]<typename Arg>(Arg arg)
            {
                if constexpr (std::same_as<Arg, urd::index_view<dg::NonTerminal<Category>, dg::ConstructorRepository>>)
                    out = fmt::format_to(out, "{}", arg);
                else
                    std::visit([&out](auto constructor) { out = fmt::format_to(out, "{}", std::pair { constructor, dl::Unparenthesized {} }); }, arg);
            },
            symbol);
    }
    return fmt::format_to(out, "))");
}

fmt::format_context::iterator
fmt::formatter<urd::index_view<dg::GrammarTag, dg::ConstructorRepository>>::format(const urd::index_view<dg::GrammarTag, dg::ConstructorRepository>& value,
                                                                                   fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(\n");
    const auto write = [&out](auto rules)
    {
        for (auto rule : rules)
            out = fmt::format_to(out, "    {}\n", rule);
    };
    write(value.get_derivation_rules<dl::ConceptTag>());
    write(value.get_derivation_rules<dl::RoleTag>());
    write(value.get_derivation_rules<dl::BooleanTag>());
    write(value.get_derivation_rules<dl::NumericalTag>());
    return fmt::format_to(out, ")\n");
}

// Each category (dl::CategoryTags).
template struct fmt::formatter<urd::index_view<dg::NonTerminal<dl::ConceptTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dg::NonTerminal<dl::RoleTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dg::NonTerminal<dl::BooleanTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dg::NonTerminal<dl::NumericalTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dg::DerivationRule<dl::ConceptTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dg::DerivationRule<dl::RoleTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dg::DerivationRule<dl::BooleanTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dg::DerivationRule<dl::NumericalTag>, dg::ConstructorRepository>>;
