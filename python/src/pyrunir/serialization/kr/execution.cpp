#include "pyrunir/serialization/binding_utils.hpp"
#include "pyrunir/module.hpp"
#include "pyrunir/serialization/bindings.hpp"

#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/serialization/kr/ps/ext/execution_view.hpp>
#include <runir/serialization/kr/ps/icp/execution_view.hpp>
#include <runir/serialization/types/execution.hpp>
#include <tyr/serialization/serialization.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace runir::serialization
{

void bind_kr_execution(nb::module_& m) { bind_entity_types(m, boost::mp11::mp_append<ExecutionViews<tyr::GroundTag>, ExecutionViews<tyr::LiftedTag>> {}); }

}  // namespace runir::serialization
