#include "bindings.hpp"

#include <runir/kr/dl/register_data.hpp>
#include <runir/kr/dl/register_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::dl
{

namespace
{

template<CategoryTag Category>
void bind_register_data(nb::module_& m, const char* name)
{
    using Type = Register<Category>;
    using Data = urd::data<Type>;

    auto data = nb::class_<Data>(m, name)
                    .def(nb::init<>())
                    .def(nb::init<urd::flat_string, RegisterIdentifier<Category>>(), nb::arg("name"), nb::arg("identifier"))
                    .def_rw("index", &Data::index)
                    .def_rw("name", &Data::name)
                    .def_rw("identifier", &Data::identifier);
    urd::python::add_comparison(data);
}

template<CategoryTag Category>
void bind_register_view(nb::module_& m, const char* name)
{
    using Type = Register<Category>;
    using View = urd::index_view<Type, ConstructorRepository>;
    auto view = nb::class_<View>(m, name).def("get_index", &View::index).def("get_name", &View::get_name).def("get_identifier", &View::get_identifier);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
}

}  // namespace

void bind_register(nb::module_& m)
{
    using Concept = Register<ConceptTag>;
    using Role = Register<RoleTag>;

    urd::python::bind_index<urd::index<Concept>>(m, "ConceptRegisterIndex");
    urd::python::bind_index<urd::index<Role>>(m, "RoleRegisterIndex");
    urd::python::bind_index<RegisterIdentifier<ConceptTag>>(m, "ConceptRegisterIdentifier");
    urd::python::bind_index<RegisterIdentifier<RoleTag>>(m, "RoleRegisterIdentifier");

    bind_register_data<ConceptTag>(m, "ConceptRegisterData");
    bind_register_data<RoleTag>(m, "RoleRegisterData");
    bind_register_view<ConceptTag>(m, "ConceptRegister");
    bind_register_view<RoleTag>(m, "RoleRegister");
}

}  // namespace runir::kr::dl
