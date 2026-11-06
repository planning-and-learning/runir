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
using StateProperties = boost::mp11::mp_list<datasets::StateGraphVertexLabel<Kind>,
                                      datasets::AnnotatedStateGraphVertexLabel<Kind>,
                                      kr::ps::base::SketchProofVertexLabel<Kind>,
                                      kr::ps::ext::ProgramProofVertexLabel<Kind>,
                                      kr::ps::icp::ProgramProofVertexLabel<Kind>>;

using GraphProperties = boost::mp11::mp_append<
    StateProperties<tyr::GroundTag>,
    StateProperties<tyr::LiftedTag>,
    boost::mp11::mp_list<datasets::StateGraphEdgeLabel, kr::ps::base::SketchProofEdgeLabel, kr::ps::ext::ProgramProofEdgeLabel, kr::ps::icp::ProgramProofEdgeLabel>>;

template<tyr::TaskKind Kind>
using StateGraphs = boost::mp11::mp_list<datasets::StaticStateGraph<Kind>,
                                  datasets::StateGraph<Kind>,
                                  datasets::StaticAnnotatedStateGraph<Kind>,
                                  datasets::AnnotatedStateGraph<Kind>,
                                  kr::ps::base::SketchProofGraph<Kind>,
                                  kr::ps::ext::ProgramProofGraph<Kind>,
                                  kr::ps::icp::ProgramProofGraph<Kind>>;

using Graphs = boost::mp11::mp_append<StateGraphs<tyr::GroundTag>, StateGraphs<tyr::LiftedTag>>;

}  // namespace runir::serialization

#endif
