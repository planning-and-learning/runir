#include "bindings.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <pyrunir/kr/dl/evaluation_bindings.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_caches.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::uns
{

namespace
{

template<typename... Types>
void bind_constructors(nb::class_<UnsConstructorRepository>& repository, ygg::TypeList<Types...>)
{
    (repository.def(
         "get_or_create",
         [](UnsConstructorRepository& self, ygg::Data<Types>& data) { return runir::kr::dl::get_or_create(self, data).first; },
         nb::arg("data"),
         nb::keep_alive<0, 1>()),
     ...);
}

template<tyr::TaskKind Kind>
void bind_state_evaluation_context(nb::module_& m, const char* name)
{
    using Context = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, Kind>;
    using Storage = semantics::EvaluationStorage<runir::kr::UnsFamilyTag>;
    using Caches = semantics::DenotationCaches<runir::kr::UnsFamilyTag>;
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
    auto repository = nb::class_<runir::kr::dl::UnsConstructorRepository>(m, "ConstructorRepository");
    repository.def("clear", &runir::kr::dl::UnsConstructorRepository::clear).def("get_index", &runir::kr::dl::UnsConstructorRepository::get_index);
    bind_constructors(repository, FamilyConstructorRepositoryTypes<runir::kr::UnsFamilyTag> {});

    auto factory = nb::class_<runir::kr::dl::UnsConstructorRepositoryFactory>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::UnsConstructorRepositoryFactory& self, tyr::formalism::planning::PlanningDomain planning_domain)
            { return self.create(planning_domain.get_repository()); },
            nb::arg("planning_domain"))
        .def(
            "create",
            [](runir::kr::dl::UnsConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::GroundTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("ground_task"))
        .def(
            "create",
            [](runir::kr::dl::UnsConstructorRepositoryFactory& self, const tyr::planning::Task<tyr::LiftedTag>& task)
            { return self.create(task.get_repository()); },
            nb::arg("lifted_task"));

    runir::kr::python::bind_evaluation_storage<runir::kr::UnsFamilyTag>(m);

    bind_state_evaluation_context<tyr::GroundTag>(m, "GroundStateEvaluationContext");
    bind_state_evaluation_context<tyr::LiftedTag>(m, "LiftedStateEvaluationContext");
}

}  // namespace runir::kr::dl::uns
