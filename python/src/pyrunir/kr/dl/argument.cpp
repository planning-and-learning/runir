#include "bindings.hpp"

#include <runir/kr/dl/argument_data.hpp>
#include <runir/kr/dl/argument_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::dl
{

namespace
{

template<CategoryTag Category>
void bind_argument_data(nb::module_& m, const char* name)
{
    using Type = Argument<Category>;
    using Data = urd::data<Type>;

    auto data = nb::class_<Data>(m, name)
                    .def(nb::init<>())
                    .def(nb::init<urd::flat_string, ArgumentIdentifier<Category>>(), nb::arg("name"), nb::arg("identifier"))
                    .def_rw("index", &Data::index)
                    .def_rw("name", &Data::name)
                    .def_rw("identifier", &Data::identifier);
    urd::python::add_comparison(data);
}

template<CategoryTag Category>
void bind_argument_view(nb::module_& m, const char* name)
{
    using Type = Argument<Category>;
    using View = urd::index_view<Type, ConstructorRepository>;
    auto view = nb::class_<View>(m, name).def("get_index", &View::index).def("get_name", &View::get_name).def("get_identifier", &View::get_identifier);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
}

}  // namespace

void bind_argument(nb::module_& m)
{
    using Concept = Argument<ConceptTag>;
    using Role = Argument<RoleTag>;
    using Boolean = Argument<BooleanTag>;
    using Numerical = Argument<NumericalTag>;

    urd::python::bind_index<urd::index<Concept>>(m, "ConceptArgumentIndex");
    urd::python::bind_index<urd::index<Role>>(m, "RoleArgumentIndex");
    urd::python::bind_index<urd::index<Boolean>>(m, "BooleanArgumentIndex");
    urd::python::bind_index<urd::index<Numerical>>(m, "NumericalArgumentIndex");
    urd::python::bind_index<ArgumentIdentifier<ConceptTag>>(m, "ConceptArgumentIdentifier");
    urd::python::bind_index<ArgumentIdentifier<RoleTag>>(m, "RoleArgumentIdentifier");
    urd::python::bind_index<ArgumentIdentifier<BooleanTag>>(m, "BooleanArgumentIdentifier");
    urd::python::bind_index<ArgumentIdentifier<NumericalTag>>(m, "NumericalArgumentIdentifier");

    bind_argument_data<ConceptTag>(m, "ConceptArgumentData");
    bind_argument_data<RoleTag>(m, "RoleArgumentData");
    bind_argument_data<BooleanTag>(m, "BooleanArgumentData");
    bind_argument_data<NumericalTag>(m, "NumericalArgumentData");
    bind_argument_view<ConceptTag>(m, "ConceptArgument");
    bind_argument_view<RoleTag>(m, "RoleArgument");
    bind_argument_view<BooleanTag>(m, "BooleanArgument");
    bind_argument_view<NumericalTag>(m, "NumericalArgument");
}

}  // namespace runir::kr::dl
