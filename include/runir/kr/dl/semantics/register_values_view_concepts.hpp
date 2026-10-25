#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_CONCEPTS_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_CONCEPTS_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"

#include <concepts>
#include <type_traits>
#include <tyr/formalism/planning/declarations.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::semantics
{

template<typename V>
concept RegisterValuesViewConcept =
    requires(const std::remove_reference_t<V>& values, RegisterIdentifier<ConceptTag> concept_id, RegisterIdentifier<RoleTag> role_id) {
        { values.get_data() } -> std::same_as<const ygg::Data<RegisterValues>&>;
        { values.get_formalism_repository() } -> std::same_as<const tyr::formalism::planning::Repository&>;
        values.template get<ConceptTag>();
        values.template get<RoleTag>();
        { values.at(concept_id).has_value() } -> std::same_as<bool>;
        { values.at(concept_id).value() } -> std::same_as<tyr::formalism::planning::ObjectView>;
        { values.at(role_id).has_value() } -> std::same_as<bool>;
        { values.at(role_id).value().get_first() } -> std::same_as<tyr::formalism::planning::ObjectView>;
        { values.at(role_id).value().get_second() } -> std::same_as<tyr::formalism::planning::ObjectView>;
    };

}  // namespace runir::kr::dl::semantics

#endif
