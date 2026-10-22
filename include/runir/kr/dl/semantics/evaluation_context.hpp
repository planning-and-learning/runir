#ifndef RUNIR_KR_DL_SEMANTICS_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"

#include <yggdrasil/database/semantics/relation_view.hpp>

namespace runir::kr::dl::semantics
{

/// Evaluate DL expressions without requiring their results to own or intern storage.
template<typename Context, typename Kind>
concept EvaluationContextConcept = tyr::TaskKind<Kind>
                                   && requires(Context& context,
                                               ConstructorView<BooleanTag> boolean,
                                               ConstructorView<NumericalTag> numerical,
                                               ConstructorView<ConceptTag> concept_,
                                               ConstructorView<RoleTag> role,
                                               QueryView<> query) {
                                          { evaluate<Kind>(boolean, context) } -> DenotationViewConcept<BooleanTag>;
                                          { evaluate<Kind>(numerical, context) } -> DenotationViewConcept<NumericalTag>;
                                          { evaluate<Kind>(concept_, context) } -> DenotationViewConcept<ConceptTag>;
                                          { evaluate<Kind>(role, context) } -> DenotationViewConcept<RoleTag>;
                                          { evaluate<Kind>(query, context) } -> ygg::database::RelationViewConcept;
                                      };

}  // namespace runir::kr::dl::semantics

#endif
