#include "module.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>
#include <runir/datasets/task_class.hpp>
#include <tyr/common/python/type_casters.hpp>

namespace runir::datasets
{

using namespace nanobind::literals;

namespace
{

template<tyr::planning::TaskKind Kind>
void bind_task_class_for_kind(nb::module_& m, const char* prefix)
{
    using TaskClassT = TaskClass<Kind>;
    using TaskSearchContextT = TaskSearchContext<Kind>;
    using TaskClassSearchContextsT = TaskClassSearchContexts<Kind>;

    nb::class_<TaskClassT>(m, (std::string(prefix) + "TaskClass").c_str())  //
        .def(nb::init<>())
        .def_rw("tasks", &TaskClassT::tasks);

    nb::class_<TaskSearchContextT>(m, (std::string(prefix) + "TaskSearchContext").c_str())  //
        .def(nb::init<>())
        .def(nb::init<tyr::planning::TaskPtr<Kind>, tyr::ExecutionContextPtr>(), "task"_a, "execution_context"_a)
        .def_rw("task", &TaskSearchContextT::task)
        .def_rw("execution_context", &TaskSearchContextT::execution_context)
        .def_rw("axiom_evaluator", &TaskSearchContextT::axiom_evaluator)
        .def_rw("state_repository", &TaskSearchContextT::state_repository)
        .def_rw("successor_generator", &TaskSearchContextT::successor_generator);

    nb::class_<TaskClassSearchContextsT>(m, (std::string(prefix) + "TaskClassSearchContexts").c_str())  //
        .def(nb::init<>())
        .def(nb::init<const TaskClassT&, tyr::ExecutionContextPtr>(), "task_class"_a, "execution_context"_a)
        .def_rw("contexts", &TaskClassSearchContextsT::contexts);
}

}  // namespace

void bind_task_class(nb::module_& m)
{
    bind_task_class_for_kind<tyr::planning::GroundTag>(m, "Ground");
    bind_task_class_for_kind<tyr::planning::LiftedTag>(m, "Lifted");
}

}  // namespace runir::datasets
