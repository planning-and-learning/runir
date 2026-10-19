#include "pyrunir/serialization/binding_utils.hpp"

#include <runir/kr/uns/formatter.hpp>
#include <runir/serialization/kr/uns/classifier_view.hpp>
#include <runir/serialization/types/uns.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace runir::serialization
{

void bind_kr_uns(nb::module_& m) { bind_entity_types(m, UnsViews {}); }

}  // namespace runir::serialization
