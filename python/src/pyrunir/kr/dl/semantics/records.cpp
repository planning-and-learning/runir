#include "../record_bindings.hpp"
#include "bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>

namespace runir::kr::dl
{

void bind_semantics_records(nb::module_& m) { python::bind_records<SemanticFamily, ConstructorRepository>(m); }

}  // namespace runir::kr::dl
