#ifndef RUNIR_KR_DL_SEMANTICS_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"

#include <concepts>
#include <ranges>
#include <yggdrasil/database/relation_view.hpp>

namespace runir::kr::dl::semantics
{

/// Evaluate DL expressions without requiring their results to own or intern storage.
template<typename Context, typename Family, typename Kind>
concept EvaluationContextConcept = runir::kr::FamilyTag<Family> && tyr::TaskKind<Kind>
                                   && requires(Context& context,
                                               runir::kr::dl::FamilyConstructorView<Family, runir::kr::dl::BooleanTag> boolean,
                                               runir::kr::dl::FamilyConstructorView<Family, runir::kr::dl::NumericalTag> numerical,
                                               runir::kr::dl::FamilyConstructorView<Family, runir::kr::dl::ConceptTag> concept_,
                                               runir::kr::dl::FamilyConstructorView<Family, runir::kr::dl::RoleTag> role,
                                               runir::kr::dl::FamilyQueryView<Family> query) {
                                          { evaluate<Kind>(boolean, context).get() } -> std::same_as<bool>;
                                          { evaluate<Kind>(numerical, context).get() } -> std::same_as<ygg::uint_t>;
                                          { evaluate<Kind>(concept_, context) } -> std::ranges::forward_range;
                                          { evaluate<Kind>(role, context) } -> std::ranges::forward_range;
                                          { evaluate<Kind>(query, context) } -> ygg::database::RelationViewConcept<ygg::Index<tyr::formalism::Object>>;
                                      };

}  // namespace runir::kr::dl::semantics

#endif
