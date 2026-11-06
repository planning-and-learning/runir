#ifndef RUNIR_CNF_GRAMMAR_SUBSTITUTION_RULE_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_SUBSTITUTION_RULE_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/non_terminal_view.hpp"
#include "runir/kr/dl/cnf_grammar/substitution_rule_data.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<runir::kr::dl::CategoryTag Category, typename V>
struct view_mixin<runir::kr::dl::cnf_grammar::SubstitutionRule<Category>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_lhs() const noexcept { return self().template get<&urd::record_of<V>::lhs>(); }
    auto get_rhs() const noexcept { return self().template get<&urd::record_of<V>::rhs>(); }
};
}  // namespace urd

#endif
