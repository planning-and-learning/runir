#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/dl/register_data.hpp>
#include <runir/kr/dl/register_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::ext
{

using namespace nanobind::literals;

namespace
{

template<CategoryTag Category>
void bind_register_data(nb::module_& m, const char* name)
{
    using Type = Register<Category>;
    using Data = ygg::Data<Type>;

    auto data = nb::class_<Data>(m, name)
                    .def(nb::init<>())
                    .def_rw("index", &Data::index)
                    .def_rw("name", &Data::name)
                    .def_rw("identifier", &Data::identifier);
    ygg::add_comparison(data);
}

template<CategoryTag Category>
void bind_register_view(nb::module_& m, const char* name)
{
    using Type = Register<Category>;
    using View = ygg::View<ygg::Index<Type>, ExtConstructorRepository>;
    auto view = nb::class_<View>(m, name)
                    .def("get_index", &View::get_index)
                    .def("get_name", &View::get_name)
                    .def("get_identifier", &View::get_identifier);
    ygg::add_comparison(view);
    ygg::add_hash(view);
}

}  // namespace

void bind_register(nb::module_& m, RepositoryBinding& repository)
{
    using Concept = Register<ConceptTag>;
    using Role = Register<RoleTag>;

    ygg::bind_index<ygg::Index<Concept>>(m, "ConceptRegisterIndex");
    ygg::bind_index<ygg::Index<Role>>(m, "RoleRegisterIndex");
    ygg::bind_index<RegisterIdentifier<ConceptTag>>(m, "ConceptRegisterIdentifier");
    ygg::bind_index<RegisterIdentifier<RoleTag>>(m, "RoleRegisterIdentifier");

    bind_register_data<ConceptTag>(m, "ConceptRegisterData");
    bind_register_data<RoleTag>(m, "RoleRegisterData");
    bind_register_view<ConceptTag>(m, "ConceptRegister");
    bind_register_view<RoleTag>(m, "RoleRegister");

    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Concept, ExtConstructorRepository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Role, ExtConstructorRepository>, "data"_a, nb::keep_alive<0, 1>());
}

}  // namespace runir::kr::dl::ext
