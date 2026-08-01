#include "bindings.hpp"

#include <runir/kr/dl/constructor_index.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::ext
{

void bind_constructor(nb::module_& m)
{
    ygg::bind_index<ygg::Index<Constructor<runir::kr::ExtFamilyTag, ConceptTag>>>(m, "ConceptConstructorIndex");
    ygg::bind_index<ygg::Index<Constructor<runir::kr::ExtFamilyTag, RoleTag>>>(m, "RoleConstructorIndex");
    ygg::bind_index<ygg::Index<Constructor<runir::kr::ExtFamilyTag, BooleanTag>>>(m, "BooleanConstructorIndex");
    ygg::bind_index<ygg::Index<Constructor<runir::kr::ExtFamilyTag, NumericalTag>>>(m, "NumericalConstructorIndex");
}

}  // namespace runir::kr::dl::ext
