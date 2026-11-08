#include "pyrunir/serialization/binding_utils.hpp"
#include "pyrunir/module.hpp"
#include "pyrunir/serialization/bindings.hpp"

#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/serialization/datasets/state_graph.hpp>
#include <runir/serialization/kr/ps/base/sketch_proof_graph.hpp>
#include <runir/serialization/kr/ps/ext/execution_view.hpp>
#include <runir/serialization/kr/ps/ext/rule_view.hpp>
#include <runir/serialization/kr/ps/icp/execution_view.hpp>
#include <runir/serialization/kr/ps/icp/rule_view.hpp>
#include <runir/serialization/kr/ps/program_proof_graph.hpp>
#include <runir/serialization/types/graphs.hpp>
#include <tyr/formalism/planning/formatter.hpp>
#include <tyr/planning/formatter.hpp>
#include <tyr/serialization/formalism/enums.hpp>
#include <tyr/serialization/formalism/planning/planning.hpp>
#include <tyr/serialization/planning/ground/task.hpp>
#include <tyr/serialization/planning/lifted/task.hpp>
#include <tyr/serialization/planning/node.hpp>
#include <tyr/serialization/planning/plan.hpp>
#include <tyr/serialization/planning/state_view.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace runir::serialization
{

void bind_graphs(nb::module_& m)
{
    bind_entity_types(m, boost::mp11::mp_append<StateProperties<tyr::GroundTag>, EdgeProperties> {});
    bind_serialized_types(m, StateGraphs<tyr::GroundTag> {});
    bind_lifted_graphs(m);
}

}  // namespace runir::serialization
