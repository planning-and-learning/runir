#include "bindings.hpp"

#include <runir/kr/dl/constructor_index.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::uns
{

void bind_constructor(nb::module_& m)
{
    ygg::bind_index<ygg::Index<Constructor<runir::kr::UnsFamilyTag, ConceptTag>>>(m, "ConceptConstructorIndex");
    ygg::bind_index<ygg::Index<Constructor<runir::kr::UnsFamilyTag, RoleTag>>>(m, "RoleConstructorIndex");
    ygg::bind_index<ygg::Index<Constructor<runir::kr::UnsFamilyTag, BooleanTag>>>(m, "BooleanConstructorIndex");
    ygg::bind_index<ygg::Index<Constructor<runir::kr::UnsFamilyTag, NumericalTag>>>(m, "NumericalConstructorIndex");
}

}  // namespace runir::kr::dl::uns
