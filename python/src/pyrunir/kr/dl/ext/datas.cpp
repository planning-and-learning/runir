#include "module.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <runir/kr/dl/grammar/datas.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::ext
{

void bind_datas(nb::module_& m)
{
    using Data = ygg::Data<runir::kr::dl::grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>>;
    nb::class_<Data>(m, "ConceptRegisterData").def(nb::init<>()).def_rw("index", &Data::index).def_rw("identifier", &Data::identifier);
}

}  // namespace runir::kr::dl::ext
