#include "pyrunir/kr/ps/ext/module.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>
#include <runir/datasets/task_class.hpp>
#include <runir/kr/ps/ext/module_program_executor.hpp>
#include <tyr/common/python/type_casters.hpp>
#include <tyr/planning/declarations.hpp>

namespace runir::kr::ps::ext
{

using namespace nanobind::literals;

namespace
{

template<tyr::planning::TaskKind Kind>
void bind_module_execution_results(nb::module_& m, const char* name)
{
    using Results = ModuleExecutionResults<Kind>;

    nb::class_<Results>(m, name)
        .def_ro("status", &Results::status)
        .def_ro("state", &Results::state)
        .def_ro("module", &Results::module)
        .def_ro("memory_state", &Results::memory_state)
        .def_ro("num_steps", &Results::num_steps)
        .def_ro("call_depth", &Results::call_depth)
        .def("is_successful", &Results::is_successful);
}

template<tyr::planning::TaskKind Kind>
void bind_module_execution_options(nb::module_& m, const char* name)
{
    using Options = ModuleExecutionOptions<Kind>;

    nb::class_<Options>(m, name)
        .def(nb::init<>())
        .def_rw("brfs_options", &Options::brfs_options)
        .def_rw("iw_options", &Options::iw_options)
        .def_rw("max_arity", &Options::max_arity)
        .def_rw("max_load_steps", &Options::max_load_steps)
        .def_rw("max_steps", &Options::max_steps);
}

}  // namespace

void bind_sketch_executor(nb::module_& m)
{
    nb::enum_<ModuleExecutionStatus>(m, "ModuleExecutionStatus")
        .value("SUCCESS", ModuleExecutionStatus::SUCCESS)
        .value("FAILURE", ModuleExecutionStatus::FAILURE)
        .value("NO_APPLICABLE_ACTION", ModuleExecutionStatus::NO_APPLICABLE_ACTION)
        .value("MALFORMED_CALL", ModuleExecutionStatus::MALFORMED_CALL)
        .value("SEARCH_FAILURE", ModuleExecutionStatus::SEARCH_FAILURE)
        .value("OUT_OF_TIME", ModuleExecutionStatus::OUT_OF_TIME)
        .value("OUT_OF_STATES", ModuleExecutionStatus::OUT_OF_STATES)
        .value("LOAD_LIMIT_REACHED", ModuleExecutionStatus::LOAD_LIMIT_REACHED)
        .value("STEP_LIMIT_REACHED", ModuleExecutionStatus::STEP_LIMIT_REACHED);

    bind_module_execution_results<tyr::planning::GroundTag>(m, "GroundModuleExecutionResults");
    bind_module_execution_results<tyr::planning::LiftedTag>(m, "LiftedModuleExecutionResults");
    bind_module_execution_options<tyr::planning::GroundTag>(m, "GroundModuleExecutionOptions");
    bind_module_execution_options<tyr::planning::LiftedTag>(m, "LiftedModuleExecutionOptions");

    m.def(
        "execute_ground_solution",
        [](const runir::datasets::TaskSearchContext<tyr::planning::GroundTag>& context,
           ModuleView entry_module,
           const ModuleExecutionOptions<tyr::planning::GroundTag>& options) { return execute_solution(context, entry_module, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "entry_module"_a,
        "options"_a = ModuleExecutionOptions<tyr::planning::GroundTag>());
    m.def(
        "execute_ground_solution",
        [](const runir::datasets::TaskSearchContext<tyr::planning::GroundTag>& context,
           ModuleView entry_module,
           const std::vector<ModuleView>& modules,
           const ModuleExecutionOptions<tyr::planning::GroundTag>& options) { return execute_solution(context, entry_module, modules, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "entry_module"_a,
        "modules"_a,
        "options"_a = ModuleExecutionOptions<tyr::planning::GroundTag>());
    m.def(
        "execute_ground_solution",
        [](const runir::datasets::TaskSearchContext<tyr::planning::GroundTag>& context,
           ModuleProgramView program,
           const ModuleExecutionOptions<tyr::planning::GroundTag>& options) { return execute_solution(context, program, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "program"_a,
        "options"_a = ModuleExecutionOptions<tyr::planning::GroundTag>());
    m.def(
        "execute_lifted_solution",
        [](const runir::datasets::TaskSearchContext<tyr::planning::LiftedTag>& context,
           ModuleView entry_module,
           const ModuleExecutionOptions<tyr::planning::LiftedTag>& options) { return execute_solution(context, entry_module, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "entry_module"_a,
        "options"_a = ModuleExecutionOptions<tyr::planning::LiftedTag>());
    m.def(
        "execute_lifted_solution",
        [](const runir::datasets::TaskSearchContext<tyr::planning::LiftedTag>& context,
           ModuleView entry_module,
           const std::vector<ModuleView>& modules,
           const ModuleExecutionOptions<tyr::planning::LiftedTag>& options) { return execute_solution(context, entry_module, modules, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "entry_module"_a,
        "modules"_a,
        "options"_a = ModuleExecutionOptions<tyr::planning::LiftedTag>());
    m.def(
        "execute_lifted_solution",
        [](const runir::datasets::TaskSearchContext<tyr::planning::LiftedTag>& context,
           ModuleProgramView program,
           const ModuleExecutionOptions<tyr::planning::LiftedTag>& options) { return execute_solution(context, program, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "program"_a,
        "options"_a = ModuleExecutionOptions<tyr::planning::LiftedTag>());
}

}  // namespace runir::kr::ps::ext
