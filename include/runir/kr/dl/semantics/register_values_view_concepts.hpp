#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_CONCEPTS_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_CONCEPTS_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"

#include <concepts>
#include <type_traits>
#include <tyr/formalism/planning/declarations.hpp>
#include <urd/elements/flat.hpp>

namespace runir::kr::dl::semantics
{

template<typename V>
concept RegisterValuesViewConcept =
    requires(const std::remove_reference_t<V>& values, RegisterIdentifier<ConceptTag> concept_id, RegisterIdentifier<RoleTag> role_id) {
        { values.data() } -> std::same_as<const urd::data<RegisterValues>&>;
        { values.get_formalism_repository() } -> std::same_as<const tyr::formalism::planning::Repository&>;
        values.template values<ConceptTag>();
        values.template values<RoleTag>();
        { values.at(concept_id).has_value() } -> std::same_as<bool>;
        requires std::same_as<std::remove_cvref_t<decltype(values.at(concept_id).value())>, tyr::formalism::planning::ObjectView>;
        { values.at(role_id).has_value() } -> std::same_as<bool>;
        requires std::same_as<std::remove_cvref_t<decltype(values.at(role_id).value().first)>, tyr::formalism::planning::ObjectView>;
        requires std::same_as<std::remove_cvref_t<decltype(values.at(role_id).value().second)>, tyr::formalism::planning::ObjectView>;
    };

}  // namespace runir::kr::dl::semantics

#endif
