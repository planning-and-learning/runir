#include "module.hpp"

#include "bindings.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/database/semantics/operations.hpp"
#include "runir/kr/database/semantics/plan_construction.hpp"
#include "runir/kr/database/syntax/formatter.hpp"
#include "yggdrasil/formalism/scratch_relation.hpp"

#include <initializer_list>
#include <nanobind/stl/vector.h>
#include <span>
#include <urd/python/bindings.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::database
{

namespace
{

/// The plans, built from their inputs' columns (validated there), and the scratch storage of the operations.
void bind_plans(nb::module_& m)
{
    nb::class_<Workspace>(m, "Workspace", "Scratch storage of the relational operations; reuse one, never concurrently.")
        .def(nb::init<>())
        .def("memory_usage", &Workspace::memory_usage);
    // The plans: records (bind_data), each made from its inputs' columns by its make_* function.
    ygg::bind_unsigned_integer<ColumnSlice>(m, "ColumnSlice");
    urd::python::bind_data<ProjectionPlan>(m);
    urd::python::bind_data<JoinPlan>(m);
    urd::python::bind_data<SetPlan>(m);
    urd::python::bind_data<SelectEqualPlan>(m);
    urd::python::bind_data<SelectValuePlan>(m);
    m.def(
        "make_projection_plan",
        [](ColumnIndices input, urd::Store<urd::sequence<Column>> labels) { return make_projection_plan(input, labels); },
        nb::arg("input"),
        nb::arg("labels"));
    m.def("make_join_plan", [](ColumnIndices left, ColumnIndices right) { return make_join_plan(left, right); }, nb::arg("left"), nb::arg("right"));
    m.def("make_set_plan", [](ColumnIndices left, ColumnIndices right) { return make_set_plan(left, right); }, nb::arg("left"), nb::arg("right"));
    m.def(
        "make_select_equal_plan",
        [](ColumnIndices input, Column left, Column right) { return make_select_equal_plan(input, left, right); },
        nb::arg("input"),
        nb::arg("left"),
        nb::arg("right"));
    m.def(
        "make_select_value_plan",
        [](ColumnIndices input, Column column, ygg::TypeId type, nb::handle value)
        { return make_select_value_plan(input, column, type, ygg::encode(ygg::cast_value(value, type))); },
        nb::arg("input"),
        nb::arg("column"),
        nb::arg("type"),
        nb::arg("value"));
}

/// The operations of one input relation type.
template<typename V>
void bind_unary(nb::module_& m)
{
    m.def(
        "project",
        [](const V& input, const urd::data<ProjectionPlan>& plan, ScratchRelation& out, Workspace& workspace)
        {
            require_columns(input.columns(), plan.input_columns);
            require_columns(out.view().columns(), plan.output_columns);
            project(input, plan, out, workspace);
        },
        nb::arg("input"),
        nb::arg("plan"),
        nb::arg("out"),
        nb::arg("workspace"));
    m.def(
        "select_equal_columns",
        [](const V& input, const urd::data<SelectEqualPlan>& plan, ScratchRelation& out)
        {
            require_columns(input.columns(), plan.output_columns);
            require_columns(out.view().columns(), plan.output_columns);
            select_equal_columns(input, plan, out);
        },
        nb::arg("input"),
        nb::arg("plan"),
        nb::arg("out"));
    m.def(
        "select_equal_value",
        [](const V& input, const urd::data<SelectValuePlan>& plan, ScratchRelation& out)
        {
            require_columns(input.columns(), plan.output_columns);
            require_columns(out.view().columns(), plan.output_columns);
            select_equal_value(input, plan, out);
        },
        nb::arg("input"),
        nb::arg("plan"),
        nb::arg("out"));
    m.def(
        "assign",
        [](nb::typed<nb::handle, ScratchRelation> destination, const V& source)
        {
            assign(nb::cast<ScratchRelation&>(destination), source);
            return destination;
        },
        nb::arg("destination"),
        nb::arg("source"));
}

/// The operations of one pair of input relation types.
template<typename L, typename R>
void bind_binary(nb::module_& m)
{
    const auto bind_join = [&m](const char* name, auto operation, bool output_is_left)
    {
        m.def(
            name,
            [operation,
             output_is_left](const L& left_operand, const R& right_operand, const urd::data<JoinPlan>& plan, ScratchRelation& out, Workspace& workspace)
            {
                require_columns(left_operand.columns(), plan.left_columns);
                require_columns(right_operand.columns(), plan.right_columns);
                require_columns(out.view().columns(), output_is_left ? plan.left_columns : plan.output_columns);
                operation(left_operand, right_operand, plan, out, workspace);
            },
            nb::arg("left_operand"),
            nb::arg("right_operand"),
            nb::arg("plan"),
            nb::arg("out"),
            nb::arg("workspace"));
    };
    bind_join("join", [](auto&&... args) { join(args...); }, false);
    bind_join("semijoin", [](auto&&... args) { semijoin(args...); }, true);
    bind_join("antijoin", [](auto&&... args) { antijoin(args...); }, true);
    const auto bind_set = [&m](const char* name, auto operation)
    {
        m.def(
            name,
            [operation](const L& left_operand, const R& right_operand, const urd::data<SetPlan>& plan, ScratchRelation& out)
            {
                for (const auto columns : { left_operand.columns(), right_operand.columns(), out.view().columns() })
                    require_columns(columns, plan.output_columns);
                operation(left_operand, right_operand, plan, out);
            },
            nb::arg("left_operand"),
            nb::arg("right_operand"),
            nb::arg("plan"),
            nb::arg("out"));
    };
    bind_set("union", [](auto&&... args) { union_(args...); });
    bind_set("difference", [](auto&&... args) { difference(args...); });
}

/// The relations the operations write and the evaluators draw: each a builder with a repository of its own rows.
void bind_scratch_relations(nb::module_& m)
{
    nb::class_<ScratchRelation>(m, "ScratchRelation", "A relation being built, its rows in tuple order in a repository of its own: an operation's output.")
        .def(
            "__init__",
            [](ScratchRelation* self)
            {
                new (self) ScratchRelation();
                self->initialize(std::span<const urd::data<ygg::ColumnLayout>>());
            },
            "The nullary relation, empty.")
        .def(
            "__init__",
            [](ScratchRelation* self, ColumnIndices columns)
            {
                new (self) ScratchRelation();
                self->initialize(columns);
            },
            nb::arg("columns"))
        .def(
            "initialize",
            [](ScratchRelation& relation, ColumnIndices columns) { relation.initialize(columns); },
            nb::arg("columns"))
        .def(
            "rename",
            [](ScratchRelation& relation, ColumnIndices columns) { relation.relation.rename(relation.rows, columns); },
            nb::arg("columns"))
        .def(
            "insert",
            [](ScratchRelation& relation, std::span<const ygg::Value> values) { return relation.insert(values).first; },
            nb::arg("values"),
            "Its position in tuple order.")
        .def(
            "erase",
            [](ScratchRelation& relation, std::span<const ygg::Value> values) { return relation.erase(values); },
            nb::arg("values"))
        .def("clear", [](ScratchRelation& relation) { relation.clear(); })
        .def("columns",
             [](const ScratchRelation& relation)
             { return std::vector<urd::data<ygg::ColumnLayout>>(relation.view().columns().begin(), relation.view().columns().end()); })
        .def("size", [](const ScratchRelation& relation) { return relation.view().get_rows().size(); })
        .def("arity", [](const ScratchRelation& relation) { return relation.view().columns().size(); })
        .def("empty", [](const ScratchRelation& relation) { return relation.view().get_rows().size() == 0; })
        .def("__len__", [](const ScratchRelation& relation) { return relation.view().get_rows().size(); })
        .def(
            "contains",
            [](const ScratchRelation& relation, std::span<const ygg::Value> values) { return relation.view().contains(values); },
            nb::arg("values"))
        .def("view", &ScratchRelation::view, nb::keep_alive<0, 1>(), "Its builder, read through its repository; the next change invalidates the view.");
    nb::class_<ygg::FreeScratchRelations>(m, "ScratchRelations", "The scratch relations an evaluator draws; they must outlive it.").def(nb::init<>());
}
}  // namespace

void bind_module_definitions(nb::module_& m)
{
    bind_scratch_relations(m);
    bind_plans(m);
    bind_unary<ygg::FreeRelationView>(m);
    bind_unary<ygg::FreeRelationBuilderView>(m);
    bind_binary<ygg::FreeRelationView, ygg::FreeRelationView>(m);
    bind_binary<ygg::FreeRelationView, ygg::FreeRelationBuilderView>(m);
    bind_binary<ygg::FreeRelationBuilderView, ygg::FreeRelationView>(m);
    bind_binary<ygg::FreeRelationBuilderView, ygg::FreeRelationBuilderView>(m);
    bind_queries(m);
    bind_optimization(m);
    bind_evaluation(m);
    bind_distance(m);
    bind_datalog(m);
}

}  // namespace runir::kr::database
