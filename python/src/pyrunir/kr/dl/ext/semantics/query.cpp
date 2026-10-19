#include "../../query_bindings.hpp"
#include "bindings.hpp"
#include <runir/kr/dl/detail/repository_instantiations_ext.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/ps/ext/detail/repository_instantiations.hpp>

namespace runir::kr::dl::ext
{

void bind_semantics_query(nb::module_& m) { python::bind_queries<runir::kr::ExtFamilyTag>(m); }

}  // namespace runir::kr::dl::ext
