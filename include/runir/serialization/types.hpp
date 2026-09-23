#ifndef RUNIR_SERIALIZATION_TYPES_HPP_
#define RUNIR_SERIALIZATION_TYPES_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/sketch_proof_graph.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/program_proof_graph.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/uns/repository.hpp"

#include <type_traits>
#include <yggdrasil/core/concepts.hpp>
#include <yggdrasil/core/type_list.hpp>

namespace runir::serialization
{

template<typename Context, typename T>
using IndexView = ygg::View<ygg::Index<T>, Context>;

using DenotationViews = ygg::ConcatTypeListsT<ygg::MapTypeListT<kr::dl::semantics::DenotationView, kr::dl::CategoryTags>,
                                              ygg::TypeList<kr::dl::semantics::RegisterValuesView, kr::dl::semantics::CallArgumentsView>>;
using BaseViews = ygg::MapTypeListSecondT<IndexView, kr::ps::base::Repository, kr::ps::base::RepositoryTypes>;
using ExtTypes =
    ygg::ConcatTypeListsT<kr::ps::ext::FeatureTypes, kr::ps::ext::ConditionTypes, kr::ps::ext::EffectTypes, kr::ps::ext::RuleTypes, kr::ps::ext::ProgramTypes>;
using ExtViews = ygg::MapTypeListSecondT<IndexView, kr::ps::ext::Repository, ExtTypes>;
using UnsViews = ygg::MapTypeListSecondT<IndexView, kr::uns::Repository, kr::uns::RepositoryTypes>;

template<tyr::TaskKind Kind>
using ExecutionViews = ygg::TypeList<kr::ps::ext::ModuleStateView<Kind>, kr::ps::ext::CallStackView<Kind>, kr::ps::ext::ProgramStateView<Kind>>;

template<tyr::TaskKind Kind>
using StateProperties = ygg::TypeList<datasets::StateGraphVertexLabel<Kind>,
                                      datasets::AnnotatedStateGraphVertexLabel<Kind>,
                                      kr::ps::base::SketchProofVertexLabel<Kind>,
                                      kr::ps::ext::ProgramProofVertexLabel<Kind>>;

using GraphProperties = ygg::ConcatTypeListsT<StateProperties<tyr::GroundTag>,
                                              StateProperties<tyr::LiftedTag>,
                                              ygg::TypeList<datasets::StateGraphEdgeLabel,
                                                            kr::ps::base::SketchProofEdgeLabel,
                                                            kr::ps::ext::ProgramProofStateTransition,
                                                            kr::ps::ext::ProgramProofEdgeLabel>>;

template<tyr::TaskKind Kind>
using StateGraphs = ygg::TypeList<datasets::StaticStateGraph<Kind>,
                                  datasets::StateGraph<Kind>,
                                  datasets::StaticAnnotatedStateGraph<Kind>,
                                  datasets::AnnotatedStateGraph<Kind>,
                                  kr::ps::base::SketchProofGraph<Kind>,
                                  kr::ps::ext::ProgramProofGraph<Kind>>;

using Graphs = ygg::ConcatTypeListsT<StateGraphs<tyr::GroundTag>, StateGraphs<tyr::LiftedTag>>;

template<typename T>
using HashableTypeList = std::conditional_t<ygg::Hashable<T>, ygg::TypeList<T>, ygg::TypeList<>>;

}  // namespace runir::serialization

#endif
