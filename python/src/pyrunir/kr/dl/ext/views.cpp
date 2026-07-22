#include "module.hpp"

#include <runir/kr/dl/repository.hpp>
#include <string>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::ext
{
namespace
{

template<typename T>
void bind_view(nb::module_& m, const std::string& name)
{
    using View = ygg::View<ygg::Index<T>, ExtConstructorRepository>;
    auto cls = nb::class_<View>(m, name.c_str())
                   .def("get_index", &View::get_index)
                   .def("get_name", &View::get_name)
                   .def("get_identifier", &View::get_identifier);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
}

}  // namespace

void bind_views(nb::module_& m)
{
    bind_view<Argument<ConceptTag>>(m, "ConceptArgument");
    bind_view<Argument<RoleTag>>(m, "RoleArgument");
    bind_view<Argument<BooleanTag>>(m, "BooleanArgument");
    bind_view<Argument<NumericalTag>>(m, "NumericalArgument");
    bind_view<Register<ConceptTag>>(m, "ConceptRegister");
    bind_view<Register<RoleTag>>(m, "RoleRegister");
}

}  // namespace runir::kr::dl::ext
