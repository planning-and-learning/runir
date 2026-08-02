#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/module_data.hpp>
#include <runir/kr/ps/ext/module_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext
{

using namespace nanobind::literals;

void bind_module(nb::module_& m, RepositoryBinding& repository)
{
    using T = Module;
    using Data = ygg::Data<T>;
    using View = ygg::View<ygg::Index<T>, Repository>;
    ygg::bind_index<ygg::Index<T>>(m, "ModuleIndex");
    auto data = nb::class_<Data>(m, "ModuleData")
                    .def(nb::init<>())
                    .def_rw("index", &Data::index)
                    .def_rw("symbol", &Data::symbol)
                    .def_rw("concept_arguments", &Data::concept_arguments)
                    .def_rw("role_arguments", &Data::role_arguments)
                    .def_rw("boolean_arguments", &Data::boolean_arguments)
                    .def_rw("numerical_arguments", &Data::numerical_arguments)
                    .def_rw("concept_registers", &Data::concept_registers)
                    .def_rw("role_registers", &Data::role_registers)
                    .def_rw("concept_features", &Data::concept_features)
                    .def_rw("boolean_features", &Data::boolean_features)
                    .def_rw("numerical_features", &Data::numerical_features)
                    .def_rw("entry_memory_state", &Data::entry_memory_state)
                    .def_rw("memory_states", &Data::memory_states)
                    .def_rw("memory_transitions", &Data::memory_transitions);
    ygg::add_comparison(data);
    auto view = nb::class_<View>(m, "Module")
                    .def("get_index", &View::get_index)
                    .def("get_name", &View::get_name)
                    .def("get_symbol", &View::get_symbol)
                    .def("get_concept_arguments", &View::template get_arguments<runir::kr::dl::ConceptTag>)
                    .def("get_role_arguments", &View::template get_arguments<runir::kr::dl::RoleTag>)
                    .def("get_boolean_arguments", &View::template get_arguments<runir::kr::dl::BooleanTag>)
                    .def("get_numerical_arguments", &View::template get_arguments<runir::kr::dl::NumericalTag>)
                    .def("get_concept_registers", &View::template get_registers<runir::kr::dl::ConceptTag>)
                    .def("get_role_registers", &View::template get_registers<runir::kr::dl::RoleTag>)
                    .def("get_concept_features", &View::template get_features<runir::kr::dl::ConceptTag>)
                    .def("get_role_features", &View::template get_features<runir::kr::dl::RoleTag>)
                    .def("get_boolean_features", &View::template get_features<runir::kr::ps::dl::BooleanFeature>)
                    .def("get_numerical_features", &View::template get_features<runir::kr::ps::dl::NumericalFeature>)
                    .def("get_entry_memory_state", &View::get_entry_memory_state, nb::keep_alive<0, 1>())
                    .def("get_memory_states", &View::get_memory_states)
                    .def("get_memory_transitions", &View::get_memory_transitions)
                    .def("syntactic_complexity", [](View value) { return runir::kr::ps::ext::syntactic_complexity(value); });
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<T, Repository>, "data"_a, nb::keep_alive<0, 1>());
}

}  // namespace runir::kr::ps::ext
