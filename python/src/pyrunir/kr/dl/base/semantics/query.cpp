#include "../../query_bindings.hpp"
#include "bindings.hpp"

namespace runir::kr::dl::base
{

void bind_semantics_query(nb::module_& m) { python::bind_queries<runir::kr::BaseFamilyTag>(m); }

}  // namespace runir::kr::dl::base
