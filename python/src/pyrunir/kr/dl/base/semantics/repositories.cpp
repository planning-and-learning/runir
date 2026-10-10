#include "bindings.hpp"

#include <memory>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <pyrunir/kr/binding_utils.hpp>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/call_arguments_view.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::base
{

namespace
{

template<typename... Types>
void bind_constructors(nb::class_<BaseConstructorRepository>& repository, ygg::TypeList<Types...>)
{
    (runir::kr::python::bind_insert<Types>(repository), ...);
}

}  // namespace

void bind_semantics_repositories(nb::module_& m)
{
    auto repository = nb::class_<runir::kr::dl::BaseConstructorRepository>(m, "ConstructorRepository");
    repository.def("clear", &runir::kr::dl::BaseConstructorRepository::clear).def("get_index", &runir::kr::dl::BaseConstructorRepository::get_index);
    bind_constructors(repository, FamilyConstructorRepositoryTypes<runir::kr::BaseFamilyTag> {});

    auto factory = nb::class_<runir::kr::dl::BaseConstructorRepositoryFactory>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::BaseConstructorRepositoryFactory& self, tyr::formalism::planning::PlanningDomain planning_domain)
            { return self.create(planning_domain.get_repository()); },
            nb::arg("planning_domain"))
        .def(
            "create",
            [](runir::kr::dl::BaseConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::GroundTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("ground_task"))
        .def(
            "create",
            [](runir::kr::dl::BaseConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::LiftedTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("lifted_task"));

    using CallArgumentsData = ygg::Data<semantics::CallArguments>;
    using RegisterValuesData = ygg::Data<semantics::RegisterValues>;
    using CallArgumentsView = semantics::CallArgumentsView;
    using RegisterValuesView = semantics::RegisterValuesView;
    using DenotationRepository = semantics::DenotationRepository;
    ygg::bind_index<ygg::Index<semantics::CallArguments>>(m, "CallArgumentsIndex");
    ygg::bind_index<ygg::Index<semantics::RegisterValues>>(m, "RegisterValuesIndex");

    using ConceptDenotation = semantics::Denotation<ConceptTag>;
    using RoleDenotation = semantics::Denotation<RoleTag>;
    using BooleanDenotation = semantics::Denotation<BooleanTag>;
    using NumericalDenotation = semantics::Denotation<NumericalTag>;
    nb::class_<CallArgumentsData>(m, "CallArgumentsData")
        .def(nb::init<>())
        .def(nb::init<ygg::IndexList<ConceptDenotation>,
                      ygg::IndexList<RoleDenotation>,
                      ygg::IndexList<BooleanDenotation>,
                      ygg::IndexList<NumericalDenotation>>(),
             nb::arg("concept_arguments"),
             nb::arg("role_arguments"),
             nb::arg("boolean_arguments"),
             nb::arg("numerical_arguments"))
        .def(nb::init<const std::vector<ygg::View<ygg::Index<ConceptDenotation>, DenotationRepository>>&,
                      const std::vector<ygg::View<ygg::Index<RoleDenotation>, DenotationRepository>>&,
                      const std::vector<ygg::View<ygg::Index<BooleanDenotation>, DenotationRepository>>&,
                      const std::vector<ygg::View<ygg::Index<NumericalDenotation>, DenotationRepository>>&>(),
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
        .def(nb::init<::cista::offset::vector<::cista::optional<ygg::Index<tyr::formalism::Object>>>,
                      ::cista::offset::vector<::cista::optional<::cista::pair<ygg::Index<tyr::formalism::Object>, ygg::Index<tyr::formalism::Object>>>>>(),
             nb::arg("concept_values"),
             nb::arg("role_values"))
        .def(nb::init<const std::vector<std::optional<tyr::formalism::planning::ObjectView>>&,
                      const std::vector<std::optional<std::pair<tyr::formalism::planning::ObjectView, tyr::formalism::planning::ObjectView>>>&>(),
             nb::arg("concept_values"),
             nb::arg("role_values"))
        .def_rw("index", &RegisterValuesData::index)
        .def_rw("concept_values", &RegisterValuesData::concept_values)
        .def_rw("role_values", &RegisterValuesData::role_values);

    const auto retainer = ygg::python::make_owner_retainer();
    using ObjectView = tyr::formalism::planning::ObjectView;
    auto register_values =
        nb::class_<RegisterValuesView>(m, "RegisterValues")
            .def("get_index", &RegisterValuesView::get_index)
            .def_prop_ro("concept_values",
                         [retainer](nb::typed<nb::handle, RegisterValuesView> owner)
                         {
                             return nb::borrow<nb::typed<nb::list, std::optional<ObjectView>>>(
                                 ygg::python::cast_with_owner(nb::cast<const RegisterValuesView&>(owner).get<ConceptTag>(), owner, retainer));
                         })
            .def_prop_ro("role_values",
                         [retainer](nb::typed<nb::handle, RegisterValuesView> owner)
                         {
                             return nb::borrow<nb::typed<nb::list, std::optional<std::pair<ObjectView, ObjectView>>>>(
                                 ygg::python::cast_with_owner(nb::cast<const RegisterValuesView&>(owner).get<RoleTag>(), owner, retainer));
                         });
    ygg::add_print(register_values);
    ygg::add_comparison(register_values);
    ygg::add_hash(register_values);

    auto call_arguments = nb::class_<CallArgumentsView>(m, "CallArguments").def("get_index", &CallArgumentsView::get_index);
    const auto bind_arguments = [&]<CategoryTag Category>(const char* name)
    {
        call_arguments.def_prop_ro(name,
                                   [retainer](nb::typed<nb::handle, CallArgumentsView> owner)
                                   {
                                       return nb::borrow<nb::typed<nb::list, semantics::DenotationView<Category>>>(
                                           ygg::python::cast_with_owner(nb::cast<const CallArgumentsView&>(owner).get<Category>(), owner, retainer));
                                   });
    };
    bind_arguments.template operator()<ConceptTag>("concept_arguments");
    bind_arguments.template operator()<RoleTag>("role_arguments");
    bind_arguments.template operator()<BooleanTag>("boolean_arguments");
    bind_arguments.template operator()<NumericalTag>("numerical_arguments");
    ygg::add_print(call_arguments);
    ygg::add_comparison(call_arguments);
    ygg::add_hash(call_arguments);

    nb::class_<runir::kr::dl::semantics::Builder>(m, "Builder").def(nb::init<>());

    runir::kr::python::bind_evaluation_storage<runir::kr::BaseFamilyTag>(m);

    auto denotations = nb::class_<DenotationRepository>(m, "DenotationRepository")
                           .def("get_index", &DenotationRepository::get_index)
                           .def(
                               "get_relation_repository",
                               [](DenotationRepository& self) -> auto& { return self.get_relation_repository(); },
                               nb::rv_policy::reference_internal)
                           .def(
                               "get_object",
                               [](const DenotationRepository& self, ygg::Index<tyr::formalism::Object> index) -> tyr::formalism::planning::ObjectView
                               { return ygg::make_view(index, self.get_formalism_repository()); },
                               nb::arg("index"),
                               nb::keep_alive<0, 1>());
    runir::kr::python::bind_insert<semantics::CallArguments>(denotations);
    runir::kr::python::bind_insert<semantics::RegisterValues>(denotations);

    nb::class_<runir::kr::dl::semantics::DenotationRepositoryFactory>(m, "DenotationRepositoryFactory")
        .def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::semantics::DenotationRepositoryFactory& self, tyr::formalism::planning::PlanningDomain planning_domain)
            { return self.create_shared(planning_domain.get_repository()); },
            nb::arg("planning_domain"));

    runir::kr::python::bind_state_evaluation_context<runir::kr::BaseFamilyTag, tyr::GroundTag>(m, "GroundStateEvaluationContext");
    runir::kr::python::bind_state_evaluation_context<runir::kr::BaseFamilyTag, tyr::LiftedTag>(m, "LiftedStateEvaluationContext");
}

}  // namespace runir::kr::dl::base
