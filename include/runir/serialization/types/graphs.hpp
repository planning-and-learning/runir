#ifndef RUNIR_SERIALIZATION_TYPES_GRAPHS_HPP_
#define RUNIR_SERIALIZATION_TYPES_GRAPHS_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/base/sketch_proof_graph.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/icp/execution_view.hpp"
#include "runir/kr/ps/program_proof_graph.hpp"
#include "runir/serialization/types/views.hpp"

#include <tyr/planning/declarations.hpp>

namespace runir::serialization
{

template<tyr::TaskKind Kind>
using StateProperties = boost::mp11::mp_list<datasets::StateGraphVertexLabel<Kind>,
                                             datasets::AnnotatedStateGraphVertexLabel<Kind>,
                                             kr::ps::base::SketchProofVertexLabel<Kind>,
                                             kr::ps::ProgramProofVertexLabel<kr::ExtFamilyTag, Kind>,
                                             kr::ps::ProgramProofVertexLabel<kr::IcpFamilyTag, Kind>>;

using EdgeProperties = boost::mp11::mp_list<datasets::StateGraphEdgeLabel,
                                            kr::ps::base::SketchProofEdgeLabel,
                                            kr::ps::ProgramProofEdgeLabel<kr::ExtFamilyTag>,
                                            kr::ps::ProgramProofEdgeLabel<kr::IcpFamilyTag>>;

template<tyr::TaskKind Kind>
using StateGraphs = boost::mp11::mp_list<datasets::StaticStateGraph<Kind>,
                                         datasets::StateGraph<Kind>,
                                         datasets::StaticAnnotatedStateGraph<Kind>,
                                         datasets::AnnotatedStateGraph<Kind>,
                                         kr::ps::base::SketchProofGraph<Kind>,
                                         kr::ps::ProgramProofGraph<kr::ExtFamilyTag, Kind>,
                                         kr::ps::ProgramProofGraph<kr::IcpFamilyTag, Kind>>;

}  // namespace runir::serialization

#endif
