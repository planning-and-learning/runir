#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/vector.h>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/module_data.hpp>
#include <runir/kr/ps/icp/module_view.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/syntactic_complexity.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::icp
{

void bind_module(nb::module_& m, RepositoryBinding& repository)
{
    using T = Module;
    using Data = urd::data<T>;
    using View = urd::index_view<T, Repository>;
    urd::python::bind_index<urd::index<T>>(m, "ModuleIndex");
    using DlRepository = runir::kr::dl::ConstructorRepository;
    auto data =
        nb::class_<Data>(m, "ModuleData")
            .def(nb::init<>())
            .def(nb::init<urd::index<ModuleSymbol<IcpFamilyTag>>,
                          urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>>,
                          urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>>,
                          urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>>,
                          urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>>>,
                          urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::BooleanTag>>>,
                          urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::NumericalTag>>>,
                          urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::QueryFeature>>>,
                          urd::index<MemoryState<IcpFamilyTag>>,
                          urd::flat_vector<urd::index<MemoryState<IcpFamilyTag>>>,
                          urd::flat_vector<urd::flat_vector<urd::index<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>>>>,
                          urd::flat_vector<ResetPair>>(),
                 nb::arg("symbol"),
                 nb::arg("concept_registers"),
                 nb::arg("role_registers"),
                 nb::arg("concept_features"),
                 nb::arg("role_features"),
                 nb::arg("boolean_features"),
                 nb::arg("numerical_features"),
                 nb::arg("query_features"),
                 nb::arg("entry_memory_state"),
                 nb::arg("memory_states"),
                 nb::arg("memory_transitions"),
                 nb::arg("reset_pairs"))
            .def(
                urd::python::init_from_views<
                    urd::index_view<ModuleSymbol<IcpFamilyTag>, Repository>,
                    const urd::Store<urd::sequence<urd::index_view<runir::kr::dl::Register<runir::kr::dl::ConceptTag>, DlRepository>>>&,
                    const urd::Store<urd::sequence<urd::index_view<runir::kr::dl::Register<runir::kr::dl::RoleTag>, DlRepository>>>&,
                    const urd::Store<urd::sequence<urd::index_view<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>, Repository>>>&,
                    const urd::Store<urd::sequence<urd::index_view<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>, Repository>>>&,
                    const urd::Store<urd::sequence<urd::index_view<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::BooleanTag>, Repository>>>&,
                    const urd::Store<urd::sequence<urd::index_view<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::NumericalTag>, Repository>>>&,
                    const urd::Store<
                        urd::sequence<urd::index_view<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::QueryFeature>, Repository>>>&,
                    urd::index_view<MemoryState<IcpFamilyTag>, Repository>,
                    const urd::Store<urd::sequence<urd::index_view<MemoryState<IcpFamilyTag>, Repository>>>&,
                    const urd::Store<urd::sequence<urd::Store<urd::sequence<urd::index_view<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>, Repository>>>>>&,
                    urd::flat_vector<ResetPair>>(),
                nb::arg("symbol"),
                nb::arg("concept_registers"),
                nb::arg("role_registers"),
                nb::arg("concept_features"),
                nb::arg("role_features"),
                nb::arg("boolean_features"),
                nb::arg("numerical_features"),
                nb::arg("query_features"),
                nb::arg("entry_memory_state"),
                nb::arg("memory_states"),
                nb::arg("memory_transitions"),
                nb::arg("reset_pairs"))
            .def_rw("index", &Data::index)
            .def_rw("symbol", &Data::symbol)
            .def_rw("concept_registers", &Data::concept_registers)
            .def_rw("role_registers", &Data::role_registers)
            .def_rw("concept_features", &Data::concept_features)
            .def_rw("role_features", &Data::role_features)
            .def_rw("boolean_features", &Data::boolean_features)
            .def_rw("numerical_features", &Data::numerical_features)
            .def_rw("query_features", &Data::query_features)
            .def_rw("entry_memory_state", &Data::entry_memory_state)
            .def_rw("memory_states", &Data::memory_states)
            .def_rw("memory_transitions", &Data::memory_transitions)
            .def_rw("reset_pairs", &Data::reset_pairs);
    urd::python::add_comparison(data);
    auto view = nb::class_<View>(m, "Module")
                    .def("index", &View::index)
                    .def("get_name", &View::get_name)
                    .def("get_symbol", &View::get_symbol)
                    .def("get_concept_registers", &View::template get_registers<runir::kr::dl::ConceptTag>)
                    .def("get_role_registers", &View::template get_registers<runir::kr::dl::RoleTag>)
                    .def("get_concept_features", &View::template get_features<runir::kr::dl::ConceptTag>)
                    .def("get_role_features", &View::template get_features<runir::kr::dl::RoleTag>)
                    .def("get_boolean_features", &View::template get_features<runir::kr::dl::BooleanTag>)
                    .def("get_numerical_features", &View::template get_features<runir::kr::dl::NumericalTag>)
                    .def("get_query_features", &View::get_query_features)
                    .def("get_entry_memory_state", &View::get_entry_memory_state)
                    .def("get_memory_states", &View::get_memory_states)
                    .def("get_memory_transitions", &View::get_memory_transitions)
                    .def("get_reset_pairs", &View::get_reset_pairs)
                    .def("syntactic_complexity", [](View value) { return runir::kr::ps::icp::syntactic_complexity(value); });
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    ygg::bind_insert<T>(repository);
}

}  // namespace runir::kr::ps::icp
