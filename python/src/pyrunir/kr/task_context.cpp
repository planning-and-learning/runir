#include "module.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/domain_context.hpp>
#include <runir/kr/task_context.hpp>

namespace runir::kr
{

using namespace nanobind::literals;

namespace
{

template<tyr::TaskKind Kind>
void bind_task_context_for_kind(nb::module_& m, const char* prefix)
{
    using TaskContextT = TaskContext<Kind>;

    nb::class_<TaskContextT>(m, (std::string(prefix) + "TaskContext").c_str())
        .def(nb::new_(&TaskContextT::create), "domain_context"_a, "search_context"_a)
        .def_prop_ro(
            "domain_context",
            [](TaskContextT& self) -> auto& { return *self.domain_context; },
            nb::rv_policy::reference_internal)
        .def_prop_ro(
            "search_context",
            [](TaskContextT& self) -> auto& { return *self.search_context; },
            nb::rv_policy::reference_internal)
        .def_prop_ro(
            "dl_builder",
            [](TaskContextT& self) -> auto& { return self.dl_builder; },
            nb::rv_policy::reference_internal)
        .def_prop_ro("dl_denotation_repository", [](TaskContextT& self) -> auto& { return *self.dl_denotation_repository; }, nb::rv_policy::reference_internal)
        .def_prop_ro(
            "execution_repository",
            [](TaskContextT& self) -> auto& { return *self.execution_repository; },
            nb::rv_policy::reference_internal)
        .def_prop_ro("execution_builder", [](TaskContextT& self) -> auto& { return self.execution_builder; }, nb::rv_policy::reference_internal);
}

}  // namespace

void bind_task_context(nb::module_& m)
{
    bind_task_context_for_kind<tyr::GroundTag>(m, "Ground");
    bind_task_context_for_kind<tyr::LiftedTag>(m, "Lifted");
}

}  // namespace runir::kr
