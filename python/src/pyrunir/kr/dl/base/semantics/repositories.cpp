#include "bindings.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <pyrunir/kr/binding_utils.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/base/state_evaluation_context.hpp>
#include <runir/kr/dl/semantics/call_arguments_view.hpp>
#include <runir/kr/dl/semantics/denotation_caches.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::base
{

namespace
{

template<tyr::TaskKind Kind>
void bind_state_evaluation_context(nb::module_& m, const char* name)
{
    using Context = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::BaseFamilyTag, Kind>;
    using DenotationCaches = runir::kr::dl::semantics::DenotationCaches<runir::kr::BaseFamilyTag>;

    nb::class_<Context>(m, name)
        .def(
            nb::new_([](tyr::planning::StateView<Kind> state,
                        runir::kr::dl::semantics::Builder& builder,
                        runir::kr::dl::semantics::DenotationRepository& denotation_repository,
                        DenotationCaches& caches)
                     { return Context(state, builder, denotation_repository, builder.get_workspace(), caches); }),
            nb::arg("state"),
            nb::arg("builder"),
            nb::arg("denotation_repository"),
            nb::arg("denotation_caches"),
            nb::keep_alive<0, 3>(),
            nb::keep_alive<0, 4>(),
            nb::keep_alive<0, 5>(),
            nb::keep_alive<5, 3>(),
            nb::keep_alive<5, 4>())
        .def("get_state", &Context::get_state, nb::rv_policy::copy, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_semantics_repositories(nb::module_& m)
{
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
                               .def_prop_ro("concept_values", &RegisterValuesView::get_concept_values)
                               .def_prop_ro("role_values", &RegisterValuesView::get_role_values);
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

    using DenotationCaches = runir::kr::dl::semantics::DenotationCaches<runir::kr::BaseFamilyTag>;
    nb::class_<DenotationCaches>(m, "DenotationCaches", "Clear dynamic denotations between contexts; clear all denotations between tasks or repositories.")
        .def(nb::init<>())
        .def("clear", nb::overload_cast<>(&DenotationCaches::clear))
        .def("clear", nb::overload_cast<bool>(&DenotationCaches::clear), nb::arg("is_static"));

    nb::class_<DenotationRepository>(m, "DenotationRepository")
        .def("get_index", &DenotationRepository::get_index)
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

    auto cls = nb::class_<runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConstructorRepository");
    cls.def("clear", &runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>::clear)
        .def("get_index", &runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>::get_index);

    auto factory = nb::class_<runir::kr::dl::ConstructorRepositoryFactoryFor<runir::kr::BaseFamilyTag>>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::ConstructorRepositoryFactoryFor<runir::kr::BaseFamilyTag>& self, tyr::formalism::planning::PlanningDomain planning_domain)
            { return self.create(planning_domain.get_repository()); },
            nb::arg("planning_domain"));
}

}  // namespace runir::kr::dl::base
