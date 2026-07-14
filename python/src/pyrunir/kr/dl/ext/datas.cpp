#include "module.hpp"

#include <runir/kr/dl/datas.hpp>
#include <string>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::ext
{
namespace
{

template<typename T>
void bind_data(nb::module_& m, const std::string& name)
{
    using Data = ygg::Data<T>;
    nb::class_<Data>(m, name.c_str())
        .def(nb::init<>())
        .def_rw("index", &Data::index)
        .def_rw("name", &Data::name)
        .def_rw("identifier", &Data::identifier);
}

}  // namespace

void bind_datas(nb::module_& m)
{
    bind_data<Argument<ConceptTag>>(m, "ConceptArgumentData");
    bind_data<Argument<RoleTag>>(m, "RoleArgumentData");
    bind_data<Argument<BooleanTag>>(m, "BooleanArgumentData");
    bind_data<Argument<NumericalTag>>(m, "NumericalArgumentData");
    bind_data<Register<ConceptTag>>(m, "ConceptRegisterData");
    bind_data<Register<RoleTag>>(m, "RoleRegisterData");
}

}  // namespace runir::kr::dl::ext
