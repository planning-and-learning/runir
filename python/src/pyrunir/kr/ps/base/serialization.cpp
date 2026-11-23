#include "pyrunir/kr/dl/serialization.hpp"

#include "module.hpp"

#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/sketch_proof_graph.hpp>
#include <tyr/formalism/planning/formatter.hpp>
#include <tyr/planning/formatter.hpp>
#include <tyr/planning/state_view.hpp>

namespace runir::kr::ps::base
{
void bind_serialization()
{
    // Its records' views, and its proof graphs and their labels.
    using Graphs = boost::mp11::mp_list<urd::data<SketchProofVertexLabel<tyr::GroundTag>>,
                                        urd::data<SketchProofVertexLabel<tyr::LiftedTag>>,
                                        urd::data<SketchProofEdgeLabel>,
                                        SketchProofGraph<tyr::GroundTag>,
                                        SketchProofGraph<tyr::LiftedTag>>;
    ygg::python::bind_serialization(boost::mp11::mp_append<ygg::python::index_views<Repository, RepositoryTypes>, Graphs> {});
}
}  // namespace runir::kr::ps::base
