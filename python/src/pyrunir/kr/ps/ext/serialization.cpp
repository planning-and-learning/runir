#include "pyrunir/kr/dl/serialization.hpp"

#include "module.hpp"

#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/ps/ext/execution_view.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/program_proof_graph.hpp>
#include <tyr/formalism/planning/formatter.hpp>
#include <tyr/planning/states/state_view.hpp>

namespace runir::kr::ps::ext
{
void bind_serialization()
{
    using ExtTypes = boost::mp11::mp_append<runir::kr::ps::ext::FeatureTypes,
                                            runir::kr::ps::ext::ConditionTypes,
                                            runir::kr::ps::ext::EffectTypes,
                                            runir::kr::ps::ext::RuleTypes,
                                            runir::kr::ps::ext::ProgramTypes>;
    // Its proof graphs and their labels besides.
    using Graphs = boost::mp11::mp_list<urd::data<runir::kr::ps::ProgramProofVertexLabel<runir::kr::ExtFamilyTag, tyr::GroundTag>>,
                                        urd::data<runir::kr::ps::ProgramProofVertexLabel<runir::kr::ExtFamilyTag, tyr::LiftedTag>>,
                                        urd::data<runir::kr::ps::ProgramProofEdgeLabel<runir::kr::ExtFamilyTag>>,
                                        runir::kr::ps::ProgramProofGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>,
                                        runir::kr::ps::ProgramProofGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>>;
    ygg::python::bind_serialization(boost::mp11::mp_append<ygg::python::index_views<runir::kr::ps::ext::Repository, ExtTypes>, Graphs> {});
}
}  // namespace runir::kr::ps::ext
