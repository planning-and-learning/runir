#include "pyrunir/serialization/binding_utils.hpp"
#include "pyrunir/serialization/bindings.hpp"

#include <runir/serialization/kr/dl/semantics/call_arguments_view.hpp>
#include <runir/serialization/kr/dl/semantics/denotation_view.hpp>
#include <runir/serialization/kr/dl/semantics/register_values_view.hpp>
#include <runir/serialization/types/dl.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace runir::serialization
{

void bind_kr_dl_denotation(nb::module_& m) { bind_entity_types(m, DenotationViews {}); }

}  // namespace runir::serialization
