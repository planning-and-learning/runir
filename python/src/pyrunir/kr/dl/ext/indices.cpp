#include "module.hpp"

#include <runir/kr/dl/grammar/ext/indices.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl::ext
{

void bind_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<runir::kr::dl::RegisterTag>>(m, "RegisterIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::ExtFamilyTag, runir::kr::dl::RegisterTag>>>(m, "ConceptRegisterIndex");
}

}  // namespace runir::kr::dl::ext
