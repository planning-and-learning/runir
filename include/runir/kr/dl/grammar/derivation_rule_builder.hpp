#ifndef RUNIR_KR_DL_GRAMMAR_DERIVATION_RULE_BUILDER_HPP_
#define RUNIR_KR_DL_GRAMMAR_DERIVATION_RULE_BUILDER_HPP_

#include "runir/kr/dl/grammar/derivation_rule_data.hpp"
#include "runir/kr/dl/grammar/derivation_rule_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of DerivationRule: its record's members, each in its built form (urd::built_t).
template<runir::kr::dl::CategoryTag Category, typename C>
struct builder<runir::kr::dl::grammar::DerivationRule<Category>, C>
{
    urd::index<runir::kr::dl::grammar::DerivationRule<Category>> index;
    urd::built_t<urd::index<runir::kr::dl::grammar::NonTerminal<Category>>, C> lhs;
    urd::built_t<urd::flat_vector<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>>, C> rhs;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"lhs", &builder::lhs>, urd::field<"rhs", &builder::rhs>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::lhs, &builder::rhs> {}; }
};

}  // namespace urd

#endif
