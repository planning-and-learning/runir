#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DECLARATIONS_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DECLARATIONS_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <urd/elements/flat.hpp>
#include <variant>

namespace runir::kr::dl::semantics::incremental
{

struct Delta;

/// Identifies a prepared result in its owning evaluation graph.
template<CategoryTag Category>
struct Evaluation
{
};

struct QueryEvaluation
{
};

template<CategoryTag Category>
using EvaluationIndex = urd::index<Evaluation<Category>>;
using QueryEvaluationIndex = urd::index<QueryEvaluation>;

/// Borrowed feature or query expression supplied when constructing a graph.
using EvaluationRoot =
    std::variant<ConstructorView<ConceptTag>, ConstructorView<RoleTag>, ConstructorView<BooleanTag>, ConstructorView<NumericalTag>, QueryView<>>;

template<tyr::TaskKind Kind>
class EvaluationGraph;

namespace detail
{
template<tyr::TaskKind Kind>
class GraphBuilder;

template<tyr::TaskKind Kind, CategoryTag Category>
class FeatureNode;

template<tyr::TaskKind Kind>
class QueryNode;
}

}  // namespace runir::kr::dl::semantics::incremental

#endif
