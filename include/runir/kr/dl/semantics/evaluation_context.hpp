#ifndef RUNIR_KR_DL_SEMANTICS_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"

#include <yggdrasil/database/semantics/relation_view.hpp>

namespace runir::kr::dl::semantics
{

/// Evaluate DL expressions without requiring their results to own or intern storage.
template<typename Context, typename Family, typename Kind>
concept EvaluationContextConcept = FamilyTag<Family> && tyr::TaskKind<Kind>
                                   && requires(Context& context,
                                               FamilyConstructorView<Family, BooleanTag> boolean,
                                               FamilyConstructorView<Family, NumericalTag> numerical,
                                               FamilyConstructorView<Family, ConceptTag> concept_,
                                               FamilyConstructorView<Family, RoleTag> role,
                                               FamilyQueryView<Family> query) {
                                          { evaluate<Kind>(boolean, context) } -> DenotationViewConcept<BooleanTag>;
                                          { evaluate<Kind>(numerical, context) } -> DenotationViewConcept<NumericalTag>;
                                          { evaluate<Kind>(concept_, context) } -> DenotationViewConcept<ConceptTag>;
                                          { evaluate<Kind>(role, context) } -> DenotationViewConcept<RoleTag>;
                                          { evaluate<Kind>(query, context) } -> ygg::database::RelationViewConcept<ObjectValues>;
                                      };

}  // namespace runir::kr::dl::semantics

#endif
