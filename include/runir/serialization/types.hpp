#ifndef RUNIR_SERIALIZATION_TYPES_HPP_
#define RUNIR_SERIALIZATION_TYPES_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/sketch_proof_graph.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/module_program_proof_graph.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/uns/repository.hpp"

#include <type_traits>
#include <tyr/serialization/types.hpp>
#include <yggdrasil/core/concepts.hpp>
#include <yggdrasil/core/type_list.hpp>

namespace runir::serialization
{

template<typename Context, typename T>
using IndexView = ygg::View<ygg::Index<T>, Context>;

template<kr::FamilyTag Family>
using DlViews = ygg::MapTypeListSecondT<IndexView, kr::dl::ConstructorRepositoryFor<Family>, kr::dl::FamilyConstructorRepositoryTypes<Family>>;

using DenotationViews = ygg::MapTypeListT<kr::dl::semantics::DenotationView, kr::dl::CategoryTags>;
using BaseViews = ygg::MapTypeListSecondT<IndexView, kr::ps::base::Repository, kr::ps::base::RepositoryTypes>;
using ExtTypes = ygg::ConcatTypeListsT<kr::ps::ext::FeatureTypes,
                                      kr::ps::ext::ConditionTypes,
                                      kr::ps::ext::EffectTypes,
                                      kr::ps::ext::RuleTypes,
                                      kr::ps::ext::ProgramTypes>;
using ExtViews = ygg::MapTypeListSecondT<IndexView, kr::ps::ext::Repository, ExtTypes>;
using UnsViews = ygg::MapTypeListSecondT<IndexView, kr::uns::Repository, kr::uns::RepositoryTypes>;

template<tyr::TaskKind Kind>
using ExecutionViews = ygg::TypeList<kr::ps::ext::RegisterValuesView<Kind>,
                                     kr::ps::ext::CallArgumentsView<Kind>,
                                     kr::ps::ext::CallStackView<Kind>,
                                     kr::ps::ext::ExecutionStateView<Kind>>;

template<tyr::TaskKind Kind>
using StateProperties = ygg::TypeList<datasets::StateGraphVertexLabel<Kind>,
                                      datasets::AnnotatedStateGraphVertexLabel<Kind>,
                                      kr::ps::base::SketchProofVertexLabel<Kind>,
                                      kr::ps::ext::ModuleProgramProofVertexLabel<Kind>>;

using GraphProperties = ygg::ConcatTypeListsT<StateProperties<tyr::GroundTag>,
                                             StateProperties<tyr::LiftedTag>,
                                             ygg::TypeList<datasets::StateGraphEdgeLabel,
                                                           kr::ps::base::SketchProofEdgeLabel,
                                                           kr::ps::ext::ModuleProgramProofStateTransition,
                                                           kr::ps::ext::ModuleProgramProofEdgeLabel>>;

template<tyr::TaskKind Kind>
using StateGraphs = ygg::TypeList<datasets::StaticStateGraph<Kind>,
                                  datasets::StateGraph<Kind>,
                                  datasets::StaticAnnotatedStateGraph<Kind>,
                                  datasets::AnnotatedStateGraph<Kind>,
                                  kr::ps::base::SketchProofGraph<Kind>,
                                  kr::ps::ext::ModuleProgramProofGraph<Kind>>;

using Graphs = ygg::ConcatTypeListsT<StateGraphs<tyr::GroundTag>, StateGraphs<tyr::LiftedTag>>;

using EntityTypes = ygg::ConcatTypeListsT<DlViews<kr::BaseFamilyTag>,
                                         DlViews<kr::ExtFamilyTag>,
                                         DlViews<kr::UnsFamilyTag>,
                                         DenotationViews,
                                         BaseViews,
                                         ExtViews,
                                         UnsViews,
                                         ExecutionViews<tyr::GroundTag>,
                                         ExecutionViews<tyr::LiftedTag>,
                                         GraphProperties>;

using SerializedTypes = ygg::ConcatTypeListsT<EntityTypes, Graphs, tyr::serialization::SerializedTypes>;

template<typename T>
using HashableTypeList = std::conditional_t<ygg::Hashable<T>, ygg::TypeList<T>, ygg::TypeList<>>;

using RegisteredTypes = ygg::ConcatTypeListsT<
    ygg::ApplyTypeListT<ygg::ConcatTypeListsT, ygg::MapTypeListT<HashableTypeList, EntityTypes>>,
    tyr::serialization::RegisteredTypes>;

using ProjectionTypes = ygg::ConcatTypeListsT<EntityTypes, Graphs, tyr::serialization::ProjectionTypes, ygg::TypeList<kr::ps::ext::ExecutionPhase>>;

}  // namespace runir::serialization

#endif
