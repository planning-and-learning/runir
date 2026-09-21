#include "../../query_bindings.hpp"
#include "bindings.hpp"

namespace runir::kr::dl::ext
{

void bind_semantics_query(nb::module_& m) { python::bind_queries<runir::kr::ExtFamilyTag>(m); }

}  // namespace runir::kr::dl::ext
