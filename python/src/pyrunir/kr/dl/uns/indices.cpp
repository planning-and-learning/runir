#include "module.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <runir/kr/dl/repository.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::uns
{

void bind_indices(nb::module_& m)
{
    // Indices into the constructor repository.
    ygg::bind_index<ygg::Index<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag>>>(m, "ConceptConstructorIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag>>>(m, "RoleConstructorIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>>(m, "BooleanConstructorIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag>>>(m, "NumericalConstructorIndex");
}

}  // namespace runir::kr::dl::uns
