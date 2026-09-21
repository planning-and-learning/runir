#include "../../query_bindings.hpp"
#include "bindings.hpp"

namespace runir::kr::dl::uns
{

void bind_semantics_query(nb::module_& m) { python::bind_queries<runir::kr::UnsFamilyTag>(m); }

}  // namespace runir::kr::dl::uns
