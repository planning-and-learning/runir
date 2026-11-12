#include "pyrunir/module.hpp"
#include "pyrunir/serialization/binding_utils.hpp"
#include "pyrunir/serialization/bindings.hpp"

#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/serialization/kr/ps/ext/execution_view.hpp>
#include <runir/serialization/kr/ps/icp/execution_view.hpp>
#include <runir/serialization/types/execution.hpp>
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

void bind_kr_execution(nb::module_& m) { bind_entity_types(m, boost::mp11::mp_append<ExecutionViews<tyr::GroundTag>, ExecutionViews<tyr::LiftedTag>> {}); }

}  // namespace runir::serialization
