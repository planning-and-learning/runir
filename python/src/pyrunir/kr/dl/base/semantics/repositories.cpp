#include "bindings.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <pyrunir/kr/binding_utils.hpp>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/call_arguments_view.hpp>
#include <runir/kr/dl/semantics/denotation_caches.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::base
{

namespace
{

template<typename... Types>
void bind_constructors(nb::class_<BaseConstructorRepository>& repository, ygg::TypeList<Types...>)
{
    (repository.def(
         "get_or_create",
         [](BaseConstructorRepository& self, ygg::Data<Types>& data) { return runir::kr::dl::get_or_create(self, data).first; },
         nb::arg("data"),
         nb::keep_alive<0, 1>()),
     ...);
}

template<tyr::TaskKind Kind>
void bind_state_evaluation_context(nb::module_& m, const char* name)
{
    using Context = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::BaseFamilyTag, Kind>;
    using Storage = semantics::EvaluationStorage<runir::kr::BaseFamilyTag>;
    using Caches = semantics::DenotationCaches<runir::kr::BaseFamilyTag>;
    nb::class_<Context>(m, name)
        .def(nb::new_([](tyr::planning::StateView<Kind> state, semantics::Builder& builder, Storage& storage) { return Context(state, builder, storage); }),
             nb::arg("state"),
             nb::arg("builder"),
             nb::arg("storage"),
             nb::keep_alive<0, 2>(),
             nb::keep_alive<0, 3>(),
             nb::keep_alive<0, 4>())
        .def(nb::new_([](tyr::planning::StateView<Kind> state,
                         semantics::Builder& builder,
                         Caches& caches,
                         semantics::DenotationRepository& repository,
                         Storage& intermediates) { return Context(state, builder, caches, repository, intermediates); }),
             nb::arg("state"),
             nb::arg("builder"),
             nb::arg("caches"),
             nb::arg("repository"),
             nb::arg("intermediates"),
             nb::keep_alive<0, 2>(),
             nb::keep_alive<0, 3>(),
             nb::keep_alive<0, 4>(),
             nb::keep_alive<0, 5>(),
             nb::keep_alive<0, 6>())
        .def("get_state", &Context::get_state, nb::rv_policy::copy, nb::keep_alive<0, 1>());
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

    nb::class_<CallArgumentsData>(m, "CallArgumentsData")
        .def(nb::init<>())
        .def_rw("concept_arguments", &CallArgumentsData::concept_arguments)
        .def_rw("role_arguments", &CallArgumentsData::role_arguments)
        .def_rw("boolean_arguments", &CallArgumentsData::boolean_arguments)
        .def_rw("numerical_arguments", &CallArgumentsData::numerical_arguments);
    nb::class_<RegisterValuesData>(m, "RegisterValuesData")
        .def(nb::init<>())
        .def_rw("concept_values", &RegisterValuesData::concept_values)
        .def_rw("role_values", &RegisterValuesData::role_values);

    auto register_values = nb::class_<RegisterValuesView>(m, "RegisterValues")
                               .def("get_index", &RegisterValuesView::get_index)
                               .def_prop_ro("concept_values", &RegisterValuesView::get<ConceptTag>)
                               .def_prop_ro("role_values", &RegisterValuesView::get<RoleTag>);
    ygg::add_print(register_values);
    ygg::add_comparison(register_values);
    ygg::add_hash(register_values);

    auto call_arguments = nb::class_<CallArgumentsView>(m, "CallArguments")
                              .def("get_index", &CallArgumentsView::get_index)
                              .def_prop_ro("concept_arguments", &CallArgumentsView::get<ConceptTag>)
                              .def_prop_ro("role_arguments", &CallArgumentsView::get<RoleTag>)
                              .def_prop_ro("boolean_arguments", &CallArgumentsView::get<BooleanTag>)
                              .def_prop_ro("numerical_arguments", &CallArgumentsView::get<NumericalTag>);
    ygg::add_print(call_arguments);
    ygg::add_comparison(call_arguments);
    ygg::add_hash(call_arguments);

    nb::class_<runir::kr::dl::semantics::Builder>(m, "Builder").def(nb::init<>());

    runir::kr::python::bind_evaluation_storage<runir::kr::BaseFamilyTag>(m);

    nb::class_<DenotationRepository>(m, "DenotationRepository")
        .def("get_index", &DenotationRepository::get_index)
        .def(
            "get_relation_repository",
            [](DenotationRepository& self) -> auto& { return self.get_relation_repository(); },
            nb::rv_policy::reference_internal)
        .def("get_or_create", &runir::kr::python::get_or_create_data<semantics::CallArguments, DenotationRepository>, nb::arg("data"), nb::keep_alive<0, 1>())
        .def("get_or_create", &runir::kr::python::get_or_create_data<semantics::RegisterValues, DenotationRepository>, nb::arg("data"), nb::keep_alive<0, 1>());

    nb::class_<runir::kr::dl::semantics::DenotationRepositoryFactory>(m, "DenotationRepositoryFactory")
        .def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::semantics::DenotationRepositoryFactory& self, tyr::formalism::planning::PlanningDomain planning_domain)
            { return self.create_shared(planning_domain.get_repository()); },
            nb::arg("planning_domain"));

    bind_state_evaluation_context<tyr::GroundTag>(m, "GroundStateEvaluationContext");
    bind_state_evaluation_context<tyr::LiftedTag>(m, "LiftedStateEvaluationContext");
}

}  // namespace runir::kr::dl::base
