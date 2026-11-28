#include "pyrunir/kr/dl/serialization.hpp"

#include "module.hpp"

#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/ps/icp/execution_view.hpp>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/reset_pair_view.hpp>
#include <runir/kr/ps/program_proof_graph.hpp>
#include <tyr/formalism/planning/formatter.hpp>
#include <tyr/planning/states/state_repository.hpp>
#include <tyr/planning/states/state_view.hpp>

namespace runir::kr::ps::icp
{
void bind_serialization()
{
    // Its records' views, the views of the values they hold (a module's reset pairs), and its proof graphs and their
    // labels.
    using Graphs = boost::mp11::mp_list<urd::data<runir::kr::ps::ProgramProofVertexLabel<runir::kr::IcpFamilyTag, tyr::GroundTag>>,
                                        urd::data<runir::kr::ps::ProgramProofVertexLabel<runir::kr::IcpFamilyTag, tyr::LiftedTag>>,
                                        urd::data<runir::kr::ps::ProgramProofEdgeLabel<runir::kr::IcpFamilyTag>>,
                                        runir::kr::ps::ProgramProofGraph<runir::kr::IcpFamilyTag, tyr::GroundTag>,
                                        runir::kr::ps::ProgramProofGraph<runir::kr::IcpFamilyTag, tyr::LiftedTag>>;
    ygg::python::bind_serialization(
        boost::mp11::mp_append<boost::mp11::mp_push_back<ygg::python::index_views<runir::kr::ps::icp::Repository, runir::kr::ps::icp::RepositoryTypes>,
                                                         runir::kr::ps::icp::ResetPairView>,
                               Graphs> {});
}
}  // namespace runir::kr::ps::icp
