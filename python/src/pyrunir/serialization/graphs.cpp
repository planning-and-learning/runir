#include "pyrunir/serialization/binding_utils.hpp"
#include "pyrunir/serialization/bindings.hpp"

#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/serialization/datasets/state_graph.hpp>
#include <runir/serialization/kr/ps/base/sketch_proof_graph.hpp>
#include <runir/serialization/kr/ps/ext/program_proof_graph.hpp>
#include <runir/serialization/kr/ps/icp/program_proof_graph.hpp>
#include <runir/serialization/types/graphs.hpp>
#include <tyr/serialization/serialization.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace runir::serialization
{

void bind_graphs(nb::module_& m)
{
    bind_entity_types(m, GraphProperties {});
    bind_serialized_types(m, Graphs {});
}

}  // namespace runir::serialization
