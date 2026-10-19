#include "../../query_bindings.hpp"
#include "bindings.hpp"
#include <runir/kr/dl/detail/repository_instantiations_uns.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/uns/detail/repository_instantiations.hpp>

namespace runir::kr::dl::uns
{

void bind_semantics_query(nb::module_& m) { python::bind_queries<runir::kr::UnsFamilyTag>(m); }

}  // namespace runir::kr::dl::uns
