#include "module.hpp"

#include <runir/kr/dl/argument_index.hpp>
#include <runir/kr/dl/constructor_index.hpp>
#include <runir/kr/dl/register_index.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::ext
{

void bind_indices(nb::module_& m)
{
    ygg::bind_index<ygg::Index<Argument<ConceptTag>>>(m, "ConceptArgumentIndex");
    ygg::bind_index<ygg::Index<Argument<RoleTag>>>(m, "RoleArgumentIndex");
    ygg::bind_index<ygg::Index<Argument<BooleanTag>>>(m, "BooleanArgumentIndex");
    ygg::bind_index<ygg::Index<Argument<NumericalTag>>>(m, "NumericalArgumentIndex");
    ygg::bind_index<ygg::Index<Register<ConceptTag>>>(m, "ConceptRegisterIndex");
    ygg::bind_index<ygg::Index<Register<RoleTag>>>(m, "RoleRegisterIndex");

    ygg::bind_index<ygg::Index<Constructor<runir::kr::ExtFamilyTag, ConceptTag>>>(m, "ConceptConstructorIndex");
    ygg::bind_index<ygg::Index<Constructor<runir::kr::ExtFamilyTag, RoleTag>>>(m, "RoleConstructorIndex");
    ygg::bind_index<ygg::Index<Constructor<runir::kr::ExtFamilyTag, BooleanTag>>>(m, "BooleanConstructorIndex");
    ygg::bind_index<ygg::Index<Constructor<runir::kr::ExtFamilyTag, NumericalTag>>>(m, "NumericalConstructorIndex");

    ygg::bind_index<ArgumentIdentifier<ConceptTag>>(m, "ConceptArgumentIdentifier");
    ygg::bind_index<ArgumentIdentifier<RoleTag>>(m, "RoleArgumentIdentifier");
    ygg::bind_index<ArgumentIdentifier<BooleanTag>>(m, "BooleanArgumentIdentifier");
    ygg::bind_index<ArgumentIdentifier<NumericalTag>>(m, "NumericalArgumentIdentifier");
    ygg::bind_index<RegisterIdentifier<ConceptTag>>(m, "ConceptRegisterIdentifier");
    ygg::bind_index<RegisterIdentifier<RoleTag>>(m, "RoleRegisterIdentifier");
}

}  // namespace runir::kr::dl::ext
