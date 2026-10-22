#include "pyrunir/serialization/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/serialization/kr/dl/semantics/constructor_view.hpp>

namespace runir::serialization
{

void bind_kr_dl_concept(nb::module_& m) { bind_entity_types(m, DlComponentViews<kr::dl::ConceptTypes> {}); }

}  // namespace runir::serialization
