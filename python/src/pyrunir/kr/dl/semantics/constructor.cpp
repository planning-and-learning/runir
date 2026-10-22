#include "bindings.hpp"

#include <nanobind/stl/variant.h>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <pyrunir/kr/dl/optimization_bindings.hpp>
#include <runir/kr/dl/detail/repository_instantiations.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/constructor_view.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/ps/ext/detail/repository_instantiations.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl
{
namespace
{

template<CategoryTag Category>
void bind_constructor_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<Constructor<Category>>;
    auto cls = nb::class_<Data>(m, name)
                   .def(nb::init<>())
                   .def(nb::init<DataVariant<Data>>(), nb::arg("variant"))
                   .def(nb::init<DataViewVariant<Data, runir::kr::dl::ConstructorRepository>>(), nb::arg("variant"))
                   .def_rw("index", &Data::index)
                   .def_rw("variant", &Data::variant)
                   .def_rw("is_static", &Data::is_static);
    ygg::add_comparison(cls);
}

template<CategoryTag Category>
void bind_constructor_view(nb::module_& m, const char* name)
{
    using Type = Constructor<Category>;
    using View = ygg::View<ygg::Index<Type>, ConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index).def("get_variant", &View::get_variant, nb::keep_alive<0, 1>());
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
    runir::kr::python::bind_evaluate(cls);
    runir::kr::python::bind_optimize(cls);
    cls.def("syntactic_complexity", [](View view) { return semantics::syntactic_complexity(view); });
    m.def("syntactic_complexity", [](View view) { return semantics::syntactic_complexity(view); }, nb::arg("constructor"));
}

}  // namespace

void bind_semantics_constructor(nb::module_& m)
{
    ygg::bind_index<ygg::Index<Constructor<ConceptTag>>>(m, "ConceptIndex");
    ygg::bind_index<ygg::Index<Constructor<RoleTag>>>(m, "RoleIndex");
    ygg::bind_index<ygg::Index<Constructor<BooleanTag>>>(m, "BooleanIndex");
    ygg::bind_index<ygg::Index<Constructor<NumericalTag>>>(m, "NumericalIndex");

    bind_constructor_data<ConceptTag>(m, "ConceptData");
    bind_constructor_data<RoleTag>(m, "RoleData");
    bind_constructor_data<BooleanTag>(m, "BooleanData");
    bind_constructor_data<NumericalTag>(m, "NumericalData");

    bind_constructor_view<ConceptTag>(m, "Concept");
    bind_constructor_view<RoleTag>(m, "Role");
    bind_constructor_view<BooleanTag>(m, "Boolean");
    bind_constructor_view<NumericalTag>(m, "Numerical");
}

}  // namespace runir::kr::dl
