#include "pyrunir/serialization/binding_utils.hpp"
#include "pyrunir/serialization/bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/serialization/kr/dl/semantics/constructor_view.hpp>

namespace runir::serialization
{

void bind_kr_dl_boolean(nb::module_& m) { bind_entity_types(m, DlComponentViews<kr::dl::BooleanTypes> {}); }

}  // namespace runir::serialization
