#ifndef RUNIR_SERIALIZATION_TYPES_GRAPHS_HPP_
#define RUNIR_SERIALIZATION_TYPES_GRAPHS_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/base/sketch_proof_graph.hpp"
#include "runir/kr/ps/ext/program_proof_graph.hpp"
#include "runir/kr/ps/icp/program_proof_graph.hpp"
#include "runir/serialization/types/views.hpp"
#include <tyr/planning/declarations.hpp>

namespace runir::serialization
{

template<tyr::TaskKind Kind>
using StateProperties = ygg::TypeList<datasets::StateGraphVertexLabel<Kind>,
                                      datasets::AnnotatedStateGraphVertexLabel<Kind>,
                                      kr::ps::base::SketchProofVertexLabel<Kind>,
                                      kr::ps::ext::ProgramProofVertexLabel<Kind>,
                                      kr::ps::icp::ProgramProofVertexLabel<Kind>>;

using GraphProperties = ygg::ConcatTypeListsT<
    StateProperties<tyr::GroundTag>,
    StateProperties<tyr::LiftedTag>,
    ygg::TypeList<datasets::StateGraphEdgeLabel, kr::ps::base::SketchProofEdgeLabel, kr::ps::ext::ProgramProofEdgeLabel, kr::ps::icp::ProgramProofEdgeLabel>>;

template<tyr::TaskKind Kind>
using StateGraphs = ygg::TypeList<datasets::StaticStateGraph<Kind>,
                                  datasets::StateGraph<Kind>,
                                  datasets::StaticAnnotatedStateGraph<Kind>,
                                  datasets::AnnotatedStateGraph<Kind>,
                                  kr::ps::base::SketchProofGraph<Kind>,
                                  kr::ps::ext::ProgramProofGraph<Kind>,
                                  kr::ps::icp::ProgramProofGraph<Kind>>;

using Graphs = ygg::ConcatTypeListsT<StateGraphs<tyr::GroundTag>, StateGraphs<tyr::LiftedTag>>;

}  // namespace runir::serialization

#endif
