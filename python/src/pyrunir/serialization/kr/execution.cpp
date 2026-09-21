#include "pyrunir/serialization/binding_utils.hpp"

namespace runir::serialization
{

void bind_kr_execution(nb::module_& m)
{
    bind_entity_types(m, ygg::ConcatTypeListsT<ExecutionViews<tyr::GroundTag>, ExecutionViews<tyr::LiftedTag>> {});
    ygg::python::bind_native_serializer<kr::ps::ext::ExecutionPhase>();
}

}  // namespace runir::serialization
