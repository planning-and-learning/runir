#include "pyrunir/serialization/binding_utils.hpp"
#include "pyrunir/serialization/bindings.hpp"

#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/serialization/kr/ps/ext/program_view.hpp>
#include <runir/serialization/types/ps_ext.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace runir::serialization
{

void bind_kr_ps_ext(nb::module_& m) { bind_entity_types(m, ExtViews {}); }

}  // namespace runir::serialization
