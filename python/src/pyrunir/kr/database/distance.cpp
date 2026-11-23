#include "runir/kr/database/semantics/distance.hpp"

#include "bindings.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/database/semantics/incremental/distance.hpp"
#include "runir/kr/database/semantics/plan_construction.hpp"

#include <nanobind/stl/pair.h>
#include <span>
#include <stdexcept>
#include <urd/python/bindings.hpp>
#include <utility>

namespace runir::kr::database
{

namespace
{

/// Binds distance and the evaluator members for one input relation type.
template<typename Input>
void bind_inputs(nb::class_<incremental::DistanceEvaluator>& evaluator, nb::module_& m)
{
    using Change = std::pair<Input, Input>;
    m.def(
        "distance",
        [](const Input& sources,
           const Input& edges,
           const Input& targets,
           const urd::data<DistancePlan>& plan,
           ScratchRelation& out,
           DistanceWorkspace& workspace)
        {
            require_columns(sources.columns(), plan.source_columns);
            require_columns(edges.columns(), plan.edge_columns);
            require_columns(targets.columns(), plan.target_columns);
            require_columns(out.view().columns(), plan.output_columns);
            if (!workspace.matches(plan))
                throw std::invalid_argument("Distance: workspace vertex size does not match the plan.");
            distance(sources, edges, targets, plan, out, workspace);
        },
        nb::arg("sources"),
        nb::arg("edges"),
        nb::arg("targets"),
        nb::arg("plan"),
        nb::arg("out"),
        nb::arg("workspace"),
        "Replaces out's rows by the reachable source/target tuples and their shortest distance, including zero-length paths.");
    evaluator
        .def("initialize", &incremental::DistanceEvaluator::template initialize<Input, Input, Input>, nb::arg("sources"), nb::arg("edges"), nb::arg("targets"))
        .def("update",
             &incremental::DistanceEvaluator::template update<Change, Change, Change>,
             nb::arg("sources"),
             nb::arg("edges"),
             nb::arg("targets"),
             "Each argument is an (added, removed) pair of actual set changes.");
}
}  // namespace

void bind_distance(nb::module_& m)
{
    urd::python::bind_data<DistancePlan>(m);
    m.def(
        "make_distance_plan",
        [](ColumnIndices source_columns, ColumnIndices edge_columns, ColumnIndices target_columns, Column distance_column)
        { return make_distance_plan(source_columns, edge_columns, target_columns, distance_column); },
        nb::arg("source_columns"),
        nb::arg("edge_columns"),
        nb::arg("target_columns"),
        nb::arg("distance_column"));
    m.def("arity", [](const urd::data<DistancePlan>& plan) { return arity(plan); }, nb::arg("plan"));

    nb::class_<DistanceWorkspace>(m, "DistanceWorkspace", "Scratch storage of distance for one plan's vertex size; reuse one, never concurrently.")
        .def(nb::init<const urd::data<DistancePlan>&>(), nb::arg("plan"))
        .def("memory_usage", &DistanceWorkspace::memory_usage);

    nb::class_<incremental::Delta>(m, "RelationDelta", "Read-only added and removed rows from the last evaluator update. The next update replaces them.")
        .def_prop_ro(
            "added",
            [](const incremental::Delta& delta) { return delta.change().first; },
            nb::keep_alive<0, 1>())
        .def_prop_ro("removed", [](const incremental::Delta& delta) { return delta.change().second; }, nb::keep_alive<0, 1>());

    auto evaluator =
        nb::class_<incremental::DistanceEvaluator>(m,
                                                   "DistanceEvaluator",
                                                   "Maintains distances and exact output deltas. Input relations are borrowed only during each call.");
    evaluator
        .def(nb::init<ygg::FreeScratchRelations&, const urd::data<DistancePlan>&>(),
             nb::arg("scratch"),
             nb::arg("plan"),
             nb::keep_alive<1, 2>(),
             nb::keep_alive<1, 3>())
        .def(
            "get_result",
            [](const incremental::DistanceEvaluator& self) { return self.get_result(); },
            nb::keep_alive<0, 1>(),
            "A view of the result builder; the next initialize or update invalidates it.")
        .def(
            "get_delta",
            [](const incremental::DistanceEvaluator& self) -> const incremental::Delta& { return self.get_delta(); },
            nb::rv_policy::reference_internal)
        .def("memory_usage", &incremental::DistanceEvaluator::memory_usage);

    bind_inputs<ygg::FreeRelationBuilderView>(evaluator, m);
    bind_inputs<ygg::FreeRelationView>(evaluator, m);
}
}  // namespace runir::kr::database
