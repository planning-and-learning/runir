#ifndef RUNIR_GRAMMAR_GRAMMAR_VIEW_HPP_
#define RUNIR_GRAMMAR_GRAMMAR_VIEW_HPP_

#include "runir/kr/dl/grammar/grammar_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/planning/domain_view.hpp>
#include <urd/repr/core/dependent_false.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::dl::grammar::GrammarTag, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_domain() const noexcept { return urd::resolve(self().data().domain, self().context().get_planning_repository()); }

    template<runir::kr::dl::CategoryTag Category>
    auto get_start() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return self().template member<&urd::record_of<V>::concept_start>();
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return self().template member<&urd::record_of<V>::role_start>();
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return self().template member<&urd::record_of<V>::boolean_start>();
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return self().template member<&urd::record_of<V>::numerical_start>();
        else
        {
            static_assert(urd::repr::dependent_false<Category>::value, "unhandled DL category in get_start");
        }
    }

    template<runir::kr::dl::CategoryTag Category>
    auto get_derivation_rules() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return self().template member<&urd::record_of<V>::concept_derivation_rules>();
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return self().template member<&urd::record_of<V>::role_derivation_rules>();
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return self().template member<&urd::record_of<V>::boolean_derivation_rules>();
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return self().template member<&urd::record_of<V>::numerical_derivation_rules>();
        else
        {
            static_assert(urd::repr::dependent_false<Category>::value, "unhandled DL category in get_derivation_rules");
        }
    }
};
}  // namespace urd

#endif
