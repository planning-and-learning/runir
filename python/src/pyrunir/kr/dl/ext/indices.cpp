#include "module.hpp"
#include "runir/kr/dl/ext/declarations.hpp"

#include <runir/kr/dl/grammar/ext/indices.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::ext
{

void bind_indices(nb::module_& m)
{
    ygg::bind_index<ygg::Index<runir::kr::dl::RegisterTag>>(m, "RegisterIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>>>(m, "ConceptRegisterIndex");
}

}  // namespace runir::kr::dl::ext
