#include "module.hpp"
#include "pyrunir/module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>
#include <runir/datasets/equivalence_graph.hpp>
#include <runir/datasets/formatter.hpp>
#include <stdexcept>
#include <string>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <urd/python/graphs.hpp>
#include <utility>
#include <vefr/dot.hpp>
#include <vefr/python/bindings.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::datasets
{

namespace
{

template<tyr::TaskKind Kind>
void bind_equivalence_graph_generation_for_kind(nb::module_& m, const char* class_prefix, const char* function_prefix)
{
    using namespace nb::literals;
    using Result = EquivalenceGraphConstructionResult<Kind>;
    using ContextList = TaskSearchContextList<Kind>;

    nb::class_<Result>(m, (std::string(class_prefix) + "EquivalenceGraphConstructionResult").c_str())
        .def_prop_ro(
            "graph",
            [](const Result& self) -> const EquivalenceGraph& { return *self.graph; },
            nb::rv_policy::reference_internal)
        .def("get_num_state_graph_results", [](const Result& self) { return self.state_graph_results.size(); })
        .def(
            "get_state_graph_result",
            [](const Result& self, std::size_t index) -> const StateGraphGenerationResult<Kind>&
            {
                if (index >= self.state_graph_results.size())
                    throw std::out_of_range("State graph result index is out of range.");
                return self.state_graph_results[index];
            },
            "index"_a,
            nb::rv_policy::reference_internal);

    m.def((std::string("generate_") + function_prefix + "_equivalence_graph").c_str(),
          [](ContextList contexts, const EquivalenceGraphGenerationOptions& options, ColorRepository& color_repository)
          { return generate_equivalence_graph<Kind>(contexts, options, color_repository); },
          "contexts"_a,
          "options"_a,
          "color_repository"_a,
          nb::keep_alive<0, 1>(),
          nb::keep_alive<0, 3>());
}

}  // namespace

void bind_equivalence_graph(nb::module_& m)
{
    using namespace nb::literals;
    nb::class_<ColorRepository>(m, "ColorRepository").def_prop_ro("num_colors", [](const ColorRepository& self) { return self.size<Color>(); });

    nb::class_<ColorRepositoryFactory>(m, "ColorRepositoryFactory")
        .def(nb::init<>())
        .def(
            "create",
            [](ColorRepositoryFactory& self, const tyr::formalism::planning::PlanningDomain& planning_domain)
            { return self.create(planning_domain.get_repository()); },
            "planning_domain"_a)
        .def(
            "create",
            [](ColorRepositoryFactory& self, const tyr::planning::Task<tyr::GroundTag>& task) { return self.create(task.get_domain().get_repository()); },
            "ground_task"_a)
        .def(
            "create",
            [](ColorRepositoryFactory& self, const tyr::planning::Task<tyr::LiftedTag>& task) { return self.create(task.get_domain().get_repository()); },
            "lifted_task"_a);

    nb::class_<EquivalenceGraphGenerationOptions>(m, "EquivalenceGraphGenerationOptions")
        .def(nb::init<>())
        .def_rw("state_graph_options", &EquivalenceGraphGenerationOptions::state_graph_options)
        .def_rw("policy_mode", &EquivalenceGraphGenerationOptions::policy_mode);

    auto vertex_label = nb::class_<EquivalenceVertexLabel>(m, "EquivalenceVertexLabel")  //
                            .def(nb::init<>())
                            .def_rw("state_graph_index", &EquivalenceVertexLabel::state_graph_index)
                            .def_rw("state_vertex_index", &EquivalenceVertexLabel::state_vertex_index);
    ygg::add_print(vertex_label);
    urd::python::add_comparison(vertex_label);
    urd::python::add_hash(vertex_label);

    auto annotated_vertex_label = nb::class_<AnnotatedEquivalenceVertexLabel>(m, "AnnotatedEquivalenceVertexLabel")  //
                                      .def(nb::init<>())
                                      .def_rw("state_graph_index", &AnnotatedEquivalenceVertexLabel::state_graph_index)
                                      .def_rw("state_vertex_index", &AnnotatedEquivalenceVertexLabel::state_vertex_index)
                                      .def_rw("goal_distance", &AnnotatedEquivalenceVertexLabel::goal_distance)
                                      .def_rw("is_initial", &AnnotatedEquivalenceVertexLabel::is_initial)
                                      .def_rw("is_goal", &AnnotatedEquivalenceVertexLabel::is_goal)
                                      .def_rw("is_alive", &AnnotatedEquivalenceVertexLabel::is_alive)
                                      .def_rw("is_unsolvable", &AnnotatedEquivalenceVertexLabel::is_unsolvable);
    ygg::add_print(annotated_vertex_label);
    urd::python::add_comparison(annotated_vertex_label);
    urd::python::add_hash(annotated_vertex_label);

    auto edge_label = nb::class_<EquivalenceEdgeLabel>(m, "EquivalenceEdgeLabel")  //
                          .def(nb::init<>())
                          .def_rw("state_graph_index", &EquivalenceEdgeLabel::state_graph_index)
                          .def_rw("state_edge_index", &EquivalenceEdgeLabel::state_edge_index);
    ygg::add_print(edge_label);
    urd::python::add_comparison(edge_label);
    urd::python::add_hash(edge_label);

    {
        auto graph = nb::class_<EquivalenceGraph>(m, "EquivalenceGraph");
        urd::python::bind_graph(graph);
        vefr::python::bind_algorithms(graph, vefr::vertex_data, vefr::edge_data);
    }
    {
        auto graph = nb::class_<AnnotatedEquivalenceGraph>(m, "AnnotatedEquivalenceGraph");
        urd::python::bind_graph(graph);
        vefr::python::bind_algorithms(graph, vefr::vertex_data, vefr::edge_data);
    }

    bind_equivalence_graph_generation_for_kind<tyr::GroundTag>(m, "Ground", "ground");
    bind_equivalence_graph_generation_for_kind<tyr::LiftedTag>(m, "Lifted", "lifted");
}

}  // namespace runir::datasets
