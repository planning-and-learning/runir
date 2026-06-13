#include "module.hpp"
#include "runir/kr/dl/ext/declarations.hpp"

#include <runir/kr/dl/ext/repository.hpp>
#include <runir/kr/dl/grammar/ext/indices.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::ext
{

void bind_indices(nb::module_& m)
{
    ygg::bind_index<ygg::Index<runir::kr::dl::RegisterTag>>(m, "RegisterIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>>>(m, "ConceptRegisterIndex");

    // Indices into the constructor repository, referenced by ext feature data/views.
    ygg::bind_index<ygg::Index<runir::kr::dl::Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>(m, "ConceptConstructorIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>>(m, "RoleConstructorIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>>(m, "BooleanConstructorIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>>(m, "NumericalConstructorIndex");

    // Register/argument identifiers are IndexMixin handles referenced by ext argument/register data.
    ygg::bind_index<runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>(m, "ConceptRegisterIdentifier");
    ygg::bind_index<runir::kr::dl::ArgumentIdentifier<runir::kr::dl::ConceptTag>>(m, "ConceptArgumentIdentifier");
    ygg::bind_index<runir::kr::dl::ArgumentIdentifier<runir::kr::dl::RoleTag>>(m, "RoleArgumentIdentifier");
    ygg::bind_index<runir::kr::dl::ArgumentIdentifier<runir::kr::dl::BooleanTag>>(m, "BooleanArgumentIdentifier");
    ygg::bind_index<runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>(m, "NumericalArgumentIdentifier");
}

}  // namespace runir::kr::dl::ext
