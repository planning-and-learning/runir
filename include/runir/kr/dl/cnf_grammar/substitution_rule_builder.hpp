#ifndef RUNIR_KR_DL_CNF_GRAMMAR_SUBSTITUTION_RULE_BUILDER_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_SUBSTITUTION_RULE_BUILDER_HPP_

#include "runir/kr/dl/cnf_grammar/substitution_rule_data.hpp"
#include "runir/kr/dl/cnf_grammar/substitution_rule_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of SubstitutionRule: its record's members, each in its built form (urd::built_t).
template<runir::kr::dl::CategoryTag Category, typename C>
struct builder<runir::kr::dl::cnf_grammar::SubstitutionRule<Category>, C>
{
    urd::index<runir::kr::dl::cnf_grammar::SubstitutionRule<Category>> index;
    urd::built_t<urd::index<runir::kr::dl::cnf_grammar::NonTerminal<Category>>, C> lhs;
    urd::built_t<urd::index<runir::kr::dl::cnf_grammar::NonTerminal<Category>>, C> rhs;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"lhs", &builder::lhs>, urd::field<"rhs", &builder::rhs>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::lhs, &builder::rhs> {}; }
};

}  // namespace urd

#endif
