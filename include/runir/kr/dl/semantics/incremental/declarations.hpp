#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DECLARATIONS_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DECLARATIONS_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <variant>
#include <yggdrasil/ids/index_mixins.hpp>

namespace runir::kr::dl::semantics::incremental
{

template<FamilyTag Family>
struct Delta;

/// Identifies a prepared result in its owning evaluation graph.
template<CategoryTag Category>
struct EvaluationIndex : ygg::IndexMixin<EvaluationIndex<Category>>
{
    using ygg::IndexMixin<EvaluationIndex<Category>>::IndexMixin;
};

struct QueryEvaluationIndex : ygg::IndexMixin<QueryEvaluationIndex>
{
    using ygg::IndexMixin<QueryEvaluationIndex>::IndexMixin;
};

/// Borrowed feature or query expression supplied when constructing a graph.
template<FamilyTag Family>
using EvaluationRoot = std::variant<FamilyConstructorView<Family, ConceptTag>,
                                    FamilyConstructorView<Family, RoleTag>,
                                    FamilyConstructorView<Family, BooleanTag>,
                                    FamilyConstructorView<Family, NumericalTag>,
                                    FamilyQueryView<Family>>;

template<FamilyTag Family, tyr::TaskKind Kind>
class EvaluationGraph;

namespace detail
{
template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category>
class FeatureNode;

template<FamilyTag Family, tyr::TaskKind Kind>
class QueryNode;
}

}  // namespace runir::kr::dl::semantics::incremental

#endif
