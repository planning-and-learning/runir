#ifndef RUNIR_CNF_GRAMMAR_GRAMMAR_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_GRAMMAR_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/grammar_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/planning/domain_view.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::dl::cnf_grammar::Grammar, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_concept_start() const noexcept { return self().template member<&urd::record_of<V>::concept_start>(); }

    auto get_role_start() const noexcept { return self().template member<&urd::record_of<V>::role_start>(); }

    auto get_boolean_start() const noexcept { return self().template member<&urd::record_of<V>::boolean_start>(); }

    auto get_numerical_start() const noexcept { return self().template member<&urd::record_of<V>::numerical_start>(); }

    auto get_concept_derivation_rules() const noexcept { return self().template member<&urd::record_of<V>::concept_derivation_rules>(); }

    auto get_role_derivation_rules() const noexcept { return self().template member<&urd::record_of<V>::role_derivation_rules>(); }

    auto get_boolean_derivation_rules() const noexcept { return self().template member<&urd::record_of<V>::boolean_derivation_rules>(); }

    auto get_numerical_derivation_rules() const noexcept { return self().template member<&urd::record_of<V>::numerical_derivation_rules>(); }

    auto get_concept_substitution_rules() const noexcept { return self().template member<&urd::record_of<V>::concept_substitution_rules>(); }

    auto get_role_substitution_rules() const noexcept { return self().template member<&urd::record_of<V>::role_substitution_rules>(); }

    auto get_boolean_substitution_rules() const noexcept { return self().template member<&urd::record_of<V>::boolean_substitution_rules>(); }

    auto get_numerical_substitution_rules() const noexcept { return self().template member<&urd::record_of<V>::numerical_substitution_rules>(); }

    auto get_domain() const noexcept { return self().template member<&urd::record_of<V>::domain>(); }

    // user: one getter templated on the tag over the fields it chooses (next to the record), besides the plain getters.
    template<runir::kr::dl::CategoryTag Category>
    auto get_start() const noexcept
    { return self().template member<runir::kr::dl::cnf_grammar::start_member<Category>()>(); }

    // user: one getter templated on the tag over the fields it chooses (next to the record), besides the plain getters.
    template<runir::kr::dl::CategoryTag Category>
    auto get_derivation_rules() const noexcept
    { return self().template member<runir::kr::dl::cnf_grammar::derivation_rules_member<Category>()>(); }

    // user: one getter templated on the tag over the fields it chooses (next to the record), besides the plain getters.
    template<runir::kr::dl::CategoryTag Category>
    auto get_substitution_rules() const noexcept
    { return self().template member<runir::kr::dl::cnf_grammar::substitution_rules_member<Category>()>(); }
};
}  // namespace urd

#endif
