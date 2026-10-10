#ifndef RUNIR_GRAMMAR_GRAMMAR_VIEW_HPP_
#define RUNIR_GRAMMAR_GRAMMAR_VIEW_HPP_

#include "runir/kr/dl/grammar/grammar_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/planning/domain_view.hpp>
#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, formalism::SymbolContextFor<runir::kr::dl::grammar::GrammarTag<Family>> C>
class View<Index<runir::kr::dl::grammar::GrammarTag<Family>>, C> : public ygg::IndexViewBase<runir::kr::dl::grammar::GrammarTag<Family>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::grammar::GrammarTag<Family>, C>::IndexViewBase;

    auto get_domain() const noexcept { return make_view(this->get_data().domain, this->get_context().get_planning_repository()); }

    template<runir::kr::dl::CategoryTag Category>
    auto get_start() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(this->get_data().concept_start, this->get_context());
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return make_view(this->get_data().role_start, this->get_context());
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return make_view(this->get_data().boolean_start, this->get_context());
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return make_view(this->get_data().numerical_start, this->get_context());
        else
        {
            static_assert(ygg::dependent_false<Category>::value, "unhandled DL category in get_start");
        }
    }

    template<runir::kr::dl::CategoryTag Category>
    auto get_derivation_rules() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(this->get_data().concept_derivation_rules, this->get_context());
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return make_view(this->get_data().role_derivation_rules, this->get_context());
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return make_view(this->get_data().boolean_derivation_rules, this->get_context());
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return make_view(this->get_data().numerical_derivation_rules, this->get_context());
        else
        {
            static_assert(ygg::dependent_false<Category>::value, "unhandled DL category in get_derivation_rules");
        }
    }
};

}  // namespace ygg

#endif
