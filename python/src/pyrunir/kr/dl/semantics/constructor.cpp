#include "bindings.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"

#include <nanobind/stl/variant.h>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <pyrunir/kr/dl/optimization_bindings.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/constructor_view.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <yggdrasil/database/syntax/query_repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::dl
{
namespace
{

template<CategoryTag Category>
void bind_constructor_data(nb::module_& m, const char* name)
{
    using Data = urd::data<Constructor<Category>>;
    auto cls = nb::class_<Data>(m, name)
                   .def(nb::init<>())
                   .def(nb::init<DataVariant<Data>>(), nb::arg("variant"))
                   .def(urd::python::init_from_views<urd::resolved_t<DataVariant<Data>, runir::kr::dl::ConstructorRepository>>(), nb::arg("variant"))
                   .def_rw("index", &Data::index)
                   .def_rw("variant", &Data::variant)
                   .def_rw("is_static", &Data::is_static);
    urd::python::add_comparison(cls);
}

template<CategoryTag Category>
void bind_constructor_view(nb::module_& m, const char* name)
{
    using Type = Constructor<Category>;
    using View = urd::index_view<Type, ConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("index", &View::index).def("get_variant", &View::get_variant);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);
    runir::kr::python::bind_evaluate(cls);
    runir::kr::python::bind_optimize(cls);
    cls.def("syntactic_complexity", [](View view) { return semantics::syntactic_complexity(view); });
    m.def("syntactic_complexity", [](View view) { return semantics::syntactic_complexity(view); }, nb::arg("constructor"));
}

}  // namespace

void bind_semantics_constructor(nb::module_& m)
{
    urd::python::bind_index<urd::index<Constructor<ConceptTag>>>(m, "ConceptIndex");
    urd::python::bind_index<urd::index<Constructor<RoleTag>>>(m, "RoleIndex");
    urd::python::bind_index<urd::index<Constructor<BooleanTag>>>(m, "BooleanIndex");
    urd::python::bind_index<urd::index<Constructor<NumericalTag>>>(m, "NumericalIndex");

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
