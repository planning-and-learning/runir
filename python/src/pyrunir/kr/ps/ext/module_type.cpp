#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <nanobind/stl/vector.h>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/module_data.hpp>
#include <runir/kr/ps/ext/module_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/syntactic_complexity.hpp>
#include <vector>
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
    using DlRepository = runir::kr::dl::ConstructorRepositoryFor<runir::kr::ps::DlFamilyFor<runir::kr::ExtFamilyTag>>;
    auto data =
        nb::class_<Data>(m, "ModuleData")
            .def(nb::init<>())
            .def(nb::init<ygg::Index<ModuleSymbol>,
                          ygg::IndexList<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>,
                          ygg::IndexList<runir::kr::dl::Argument<runir::kr::dl::RoleTag>>,
                          ygg::IndexList<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>,
                          ygg::IndexList<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>,
                          ygg::IndexList<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>,
                          ygg::IndexList<runir::kr::dl::Register<runir::kr::dl::RoleTag>>,
                          ygg::IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>,
                          ygg::IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>,
                          ygg::IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>,
                          ygg::IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>,
                          ygg::IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>,
                          ygg::Index<MemoryState>,
                          ygg::IndexList<MemoryState>,
                          ygg::IndexMatrix<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>>(),
                 nb::arg("symbol"),
                 nb::arg("concept_arguments"),
                 nb::arg("role_arguments"),
                 nb::arg("boolean_arguments"),
                 nb::arg("numerical_arguments"),
                 nb::arg("concept_registers"),
                 nb::arg("role_registers"),
                 nb::arg("concept_features"),
                 nb::arg("role_features"),
                 nb::arg("boolean_features"),
                 nb::arg("numerical_features"),
                 nb::arg("query_features"),
                 nb::arg("entry_memory_state"),
                 nb::arg("memory_states"),
                 nb::arg("memory_transitions"))
            .def(
                nb::init<
                    ygg::View<ygg::Index<ModuleSymbol>, Repository>,
                    const std::vector<ygg::View<ygg::Index<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>, DlRepository>>&,
                    const std::vector<ygg::View<ygg::Index<runir::kr::dl::Argument<runir::kr::dl::RoleTag>>, DlRepository>>&,
                    const std::vector<ygg::View<ygg::Index<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>, DlRepository>>&,
                    const std::vector<ygg::View<ygg::Index<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>, DlRepository>>&,
                    const std::vector<ygg::View<ygg::Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>, DlRepository>>&,
                    const std::vector<ygg::View<ygg::Index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>, DlRepository>>&,
                    const std::vector<ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>, Repository>>&,
                    const std::vector<ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>, Repository>>&,
                    const std::vector<ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>, Repository>>&,
                    const std::vector<ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>, Repository>>&,
                    const std::vector<ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>, Repository>>&,
                    ygg::View<ygg::Index<MemoryState>, Repository>,
                    const std::vector<ygg::View<ygg::Index<MemoryState>, Repository>>&,
                    const std::vector<std::vector<ygg::View<ygg::Index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>, Repository>>>&>(),
                nb::arg("symbol"),
                nb::arg("concept_arguments"),
                nb::arg("role_arguments"),
                nb::arg("boolean_arguments"),
                nb::arg("numerical_arguments"),
                nb::arg("concept_registers"),
                nb::arg("role_registers"),
                nb::arg("concept_features"),
                nb::arg("role_features"),
                nb::arg("boolean_features"),
                nb::arg("numerical_features"),
                nb::arg("query_features"),
                nb::arg("entry_memory_state"),
                nb::arg("memory_states"),
                nb::arg("memory_transitions"))
            .def_rw("index", &Data::index)
            .def_rw("symbol", &Data::symbol)
            .def_rw("concept_arguments", &Data::concept_arguments)
            .def_rw("role_arguments", &Data::role_arguments)
            .def_rw("boolean_arguments", &Data::boolean_arguments)
            .def_rw("numerical_arguments", &Data::numerical_arguments)
            .def_rw("concept_registers", &Data::concept_registers)
            .def_rw("role_registers", &Data::role_registers)
            .def_rw("concept_features", &Data::concept_features)
            .def_rw("role_features", &Data::role_features)
            .def_rw("boolean_features", &Data::boolean_features)
            .def_rw("numerical_features", &Data::numerical_features)
            .def_rw("query_features", &Data::query_features)
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
                    .def("get_boolean_features", &View::template get_features<runir::kr::dl::BooleanTag>)
                    .def("get_numerical_features", &View::template get_features<runir::kr::dl::NumericalTag>)
                    .def("get_query_features", &View::get_query_features)
                    .def("get_entry_memory_state", &View::get_entry_memory_state, nb::keep_alive<0, 1>())
                    .def("get_memory_states", &View::get_memory_states)
                    .def("get_memory_transitions", &View::get_memory_transitions)
                    .def("syntactic_complexity", [](View value) { return runir::kr::ps::ext::syntactic_complexity(value); });
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    runir::kr::python::bind_insert<T>(repository);
}

}  // namespace runir::kr::ps::ext
