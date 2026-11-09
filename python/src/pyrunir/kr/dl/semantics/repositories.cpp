#include "bindings.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/dl/semantics/detail/repository_instantiations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/python/sequences/sequence.hpp>

#include <memory>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <pyrunir/kr/binding_utils.hpp>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/call_arguments_view.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::dl
{

namespace
{

template<typename... Types>
void bind_constructors(nb::class_<ConstructorRepository>& repository, boost::mp11::mp_list<Types...>)
{ (ygg::bind_insert<Types>(repository), ...); }

}  // namespace

void bind_semantics_repositories(nb::module_& m)
{
    using CallArgumentsData = urd::data<semantics::CallArguments>;
    using RegisterValuesData = urd::data<semantics::RegisterValues>;
    using CallArgumentsView = semantics::CallArgumentsView;
    using RegisterValuesView = semantics::RegisterValuesView;
    using DenotationRepository = semantics::DenotationRepository;
    urd::python::bind_index<urd::index<semantics::CallArguments>>(m, "CallArgumentsIndex");
    urd::python::bind_index<urd::index<semantics::RegisterValues>>(m, "RegisterValuesIndex");

    using ConceptDenotation = semantics::Denotation<ConceptTag>;
    using RoleDenotation = semantics::Denotation<RoleTag>;
    using BooleanDenotation = semantics::Denotation<BooleanTag>;
    using NumericalDenotation = semantics::Denotation<NumericalTag>;
    nb::class_<CallArgumentsData>(m, "CallArgumentsData")
        .def(nb::init<>())
        .def(nb::init<urd::flat_vector<urd::index<ConceptDenotation>>,
                      urd::flat_vector<urd::index<RoleDenotation>>,
                      urd::flat_vector<urd::index<BooleanDenotation>>,
                      urd::flat_vector<urd::index<NumericalDenotation>>>(),
             nb::arg("concept_arguments"),
             nb::arg("role_arguments"),
             nb::arg("boolean_arguments"),
             nb::arg("numerical_arguments"))
        .def(urd::python::init_from_views<const urd::Store<urd::sequence<urd::index_view<ConceptDenotation, DenotationRepository>>>&,
                      const urd::Store<urd::sequence<urd::index_view<RoleDenotation, DenotationRepository>>>&,
                      const urd::Store<urd::sequence<urd::index_view<BooleanDenotation, DenotationRepository>>>&,
                      const urd::Store<urd::sequence<urd::index_view<NumericalDenotation, DenotationRepository>>>&>(),
             nb::arg("concept_arguments"),
             nb::arg("role_arguments"),
             nb::arg("boolean_arguments"),
             nb::arg("numerical_arguments"))
        .def_rw("index", &CallArgumentsData::index)
        .def_rw("concept_arguments", &CallArgumentsData::concept_arguments)
        .def_rw("role_arguments", &CallArgumentsData::role_arguments)
        .def_rw("boolean_arguments", &CallArgumentsData::boolean_arguments)
        .def_rw("numerical_arguments", &CallArgumentsData::numerical_arguments);
    nb::class_<RegisterValuesData>(m, "RegisterValuesData")
        .def(nb::init<>())
        .def(nb::init<urd::flat_vector<urd::flat_optional<urd::index<tyr::formalism::Object>>>,
                      urd::flat_vector<urd::flat_optional<urd::flat_pair<urd::index<tyr::formalism::Object>, urd::index<tyr::formalism::Object>>>>>(),
             nb::arg("concept_values"),
             nb::arg("role_values"))
        .def(urd::python::init_from_views<const urd::Store<urd::sequence<std::optional<tyr::formalism::planning::ObjectView>>>&,
                      const urd::Store<urd::sequence<std::optional<std::pair<tyr::formalism::planning::ObjectView, tyr::formalism::planning::ObjectView>>>>&>(),
             nb::arg("concept_values"),
             nb::arg("role_values"))
        .def_rw("index", &RegisterValuesData::index)
        .def_rw("concept_values", &RegisterValuesData::concept_values)
        .def_rw("role_values", &RegisterValuesData::role_values);

    using ObjectView = tyr::formalism::planning::ObjectView;
    auto register_values =
        nb::class_<RegisterValuesView>(m, "RegisterValues")
            .def("index", &RegisterValuesView::index)
            .def_prop_ro("concept_values", [](const RegisterValuesView& self) { return self.values<ConceptTag>(); })
            .def_prop_ro("role_values", [](const RegisterValuesView& self) { return self.values<RoleTag>(); });
    ygg::add_print(register_values);
    urd::python::add_comparison(register_values);
    urd::python::add_hash(register_values);

    auto call_arguments = nb::class_<CallArgumentsView>(m, "CallArguments").def("index", &CallArgumentsView::index);
    const auto bind_arguments = [&]<CategoryTag Category>(const char* name)
    {
        call_arguments.def_prop_ro(name, [](const CallArgumentsView& self) { return self.template arguments<Category>(); });
    };
    bind_arguments.template operator()<ConceptTag>("concept_arguments");
    bind_arguments.template operator()<RoleTag>("role_arguments");
    bind_arguments.template operator()<BooleanTag>("boolean_arguments");
    bind_arguments.template operator()<NumericalTag>("numerical_arguments");
    ygg::add_print(call_arguments);
    urd::python::add_comparison(call_arguments);
    urd::python::add_hash(call_arguments);

    nb::class_<runir::kr::dl::semantics::Builder>(m, "Builder").def(nb::init<>());

    auto denotations = nb::class_<DenotationRepository>(m, "DenotationRepository")
                           .def("layer", &DenotationRepository::layer)
                           .def(
                               "get_relation_repository",
                               [](DenotationRepository& self) -> auto& { return self.get_relation_repository(); },
                               nb::rv_policy::reference_internal)
                           .def(
                               "get_object",
                               [](const DenotationRepository& self, urd::index<tyr::formalism::Object> index) -> tyr::formalism::planning::ObjectView
                               { return self.get_formalism_repository().view(index); },
                               nb::arg("index"));
    ygg::bind_insert<semantics::CallArguments>(denotations);
    ygg::bind_insert<semantics::RegisterValues>(denotations);

    nb::class_<runir::kr::dl::semantics::DenotationRepositoryFactory>(m, "DenotationRepositoryFactory")
        .def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::semantics::DenotationRepositoryFactory& self, const tyr::formalism::planning::PlanningDomain& planning_domain)
            { return self.create_shared(planning_domain.get_repository()); },
            nb::arg("planning_domain"));

    runir::kr::python::bind_evaluation_storage(m);
    runir::kr::python::bind_state_evaluation_context<tyr::GroundTag>(m, "GroundStateEvaluationContext");
    runir::kr::python::bind_state_evaluation_context<tyr::LiftedTag>(m, "LiftedStateEvaluationContext");

    auto repository = nb::class_<runir::kr::dl::ConstructorRepository>(m, "ConstructorRepository");
    repository.def("clear", &runir::kr::dl::ConstructorRepository::clear).def("layer", &runir::kr::dl::ConstructorRepository::layer);
    bind_constructors(repository, ConstructorRepositoryTypes {});

    auto factory = nb::class_<runir::kr::dl::ConstructorRepositoryFactory>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::ConstructorRepositoryFactory& self, const tyr::formalism::planning::PlanningDomain& planning_domain)
            { return self.create(planning_domain.get_repository()); },
            nb::arg("planning_domain"))
        .def(
            "create",
            [](runir::kr::dl::ConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::GroundTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("ground_task"))
        .def(
            "create",
            [](runir::kr::dl::ConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::LiftedTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("lifted_task"));
}

}  // namespace runir::kr::dl
