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
    auto get_domain() const noexcept { return urd::resolve(self().data().domain, self().context().get_planning_repository()); }

    template<runir::kr::dl::CategoryTag Category>
    auto get_start() const noexcept
    { return urd::resolve(self().data().template get_start<Category>(), self().context()); }

    template<runir::kr::dl::CategoryTag Category>
    auto get_derivation_rules() const noexcept
    { return urd::resolve(self().data().template get_derivation_rules<Category>(), self().context()); }

    template<runir::kr::dl::CategoryTag Category>
    auto get_substitution_rules() const noexcept
    { return urd::resolve(self().data().template get_substitution_rules<Category>(), self().context()); }
};
}  // namespace urd

#endif
