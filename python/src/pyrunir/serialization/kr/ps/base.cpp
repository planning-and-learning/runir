#include "pyrunir/serialization/binding_utils.hpp"
#include "pyrunir/serialization/bindings.hpp"

#include <runir/kr/ps/base/formatter.hpp>
#include <runir/serialization/kr/ps/base/sketch_view.hpp>
#include <runir/serialization/types/ps_base.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace runir::serialization
{

void bind_kr_ps_base(nb::module_& m) { bind_entity_types(m, BaseViews {}); }

}  // namespace runir::serialization
