#include "pyrunir/serialization/binding_utils.hpp"

#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/serialization/kr/ps/icp/program_view.hpp>
#include <runir/serialization/types/ps_icp.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace runir::serialization
{

void bind_kr_ps_icp(nb::module_& m) { bind_entity_types(m, IcpViews {}); }

}  // namespace runir::serialization
