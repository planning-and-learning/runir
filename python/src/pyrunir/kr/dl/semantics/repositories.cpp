#include "module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation_context.hpp>
#include <runir/kr/dl/repository.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground_task/state_repository.hpp>
#include <tyr/planning/ground_task/state_view.hpp>
#include <tyr/planning/lifted_task/state_repository.hpp>
#include <tyr/planning/lifted_task/state_view.hpp>

namespace runir::kr::dl
{

namespace
{

template<tyr::planning::TaskKind Kind>
void bind_evaluation_context(nb::module_& m, const char* name)
{
    using Context = runir::kr::dl::semantics::EvaluationContext<Kind>;

    nb::class_<Context>(m, name)
        .def(nb::init<tyr::planning::StateView<Kind>, runir::kr::dl::semantics::Builder&, runir::kr::dl::semantics::DenotationRepository&>(),
             nb::arg("state"),
             nb::arg("builder"),
             nb::arg("denotation_repository"),
             nb::keep_alive<1, 3>(),
             nb::keep_alive<1, 4>())
        .def("get_state", &Context::get_state, nb::rv_policy::reference_internal);
}

}  // namespace

void bind_semantics_repositories(nb::module_& m)
{
    nb::class_<runir::kr::dl::semantics::Builder>(m, "Builder").def(nb::init<>());

    nb::class_<runir::kr::dl::semantics::DenotationRepository>(m, "DenotationRepository").def("get_index", &runir::kr::dl::semantics::DenotationRepository::get_index);

    nb::class_<runir::kr::dl::semantics::DenotationRepositoryFactory>(m, "DenotationRepositoryFactory")
        .def(nb::init<>())
        .def("create", &runir::kr::dl::semantics::DenotationRepositoryFactory::create_shared);

    bind_evaluation_context<tyr::planning::GroundTag>(m, "GroundEvaluationContext");
    bind_evaluation_context<tyr::planning::LiftedTag>(m, "LiftedEvaluationContext");

    auto cls = nb::class_<runir::kr::dl::ConstructorRepository>(m, "ConstructorRepository");
    cls.def("clear", &runir::kr::dl::ConstructorRepository::clear).def("get_index", &runir::kr::dl::ConstructorRepository::get_index);

    auto factory = nb::class_<runir::kr::dl::ConstructorRepositoryFactory>(m, "ConstructorRepositoryFactory");
    factory.def(nb::init<>())
        .def(
            "create",
            [](runir::kr::dl::ConstructorRepositoryFactory& self, tyr::formalism::planning::PlanningDomain planning_domain)
            { return self.create_shared(planning_domain.get_repository()); },
            nb::arg("planning_domain"));
}

}  // namespace runir::kr::dl
