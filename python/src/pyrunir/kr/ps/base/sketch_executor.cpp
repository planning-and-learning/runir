#include "pyrunir/kr/ps/base/module.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>
#include <pyrunir/graphs/graph.hpp>
#include <runir/datasets/state_graph.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/sketch_executor.hpp>
#include <runir/kr/ps/base/successor_expander.hpp>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::base
{

using namespace nanobind::literals;
using runir::graphs::bind_forward_graph;
using runir::graphs::bind_readable_graph_methods;

namespace
{
template<tyr::planning::TaskKind Kind>
void bind_sketch_proof_types(nb::module_& m, const char* prefix)
{
    using Graph = SketchProofGraph<Kind>;
    using Results = SketchProofResults<Kind>;

    auto graph = nb::class_<Graph>(m, (std::string(prefix) + "SketchProofGraph").c_str());
    graph.def(nb::init<>());
    bind_readable_graph_methods(graph);
    bind_forward_graph(graph);

    nb::class_<Results>(m, (std::string(prefix) + "SketchProofResults").c_str())
        .def_ro("status", &Results::status)
        .def_ro("graph", &Results::graph)
        .def_ro("deadend_transitions", &Results::deadend_transitions)
        .def_ro("open_states", &Results::open_states)
        .def_ro("cycle", &Results::cycle)
        .def("is_successful", &Results::is_successful);
}

template<tyr::planning::TaskKind Kind>
void bind_sketch_search_options(nb::module_& m, const char* name)
{
    using Options = SketchSearchOptions<Kind>;

    nb::class_<Options>(m, name)
        .def(nb::init<>())
        .def_rw("brfs_options", &Options::brfs_options)
        .def_rw("iw_options", &Options::iw_options)
        .def_rw("siw_options", &Options::siw_options)
        .def_rw("max_arity", &Options::max_arity);
}


template<tyr::planning::TaskKind Kind>
struct BoundExecutionContext
{
    runir::kr::ps::base::EvaluationContext<Kind> context;
    bool is_initial;
};

template<tyr::planning::TaskKind Kind>
std::string context_key(const BoundExecutionContext<Kind>& bound)
{
    return "s" + std::to_string(bound.context.get_state().get_index().get_value());
}

template<tyr::planning::TaskKind Kind>
class BoundSuccessorExpander
{
private:
    runir::kr::dl::semantics::Builder m_dl_builder;
    runir::kr::dl::semantics::DenotationRepositoryFactory m_dl_denotation_repository_factory;
    runir::kr::dl::semantics::DenotationRepository m_dl_denotation_repository;
    SuccessorExpander<Kind> m_expander;

public:
    explicit BoundSuccessorExpander(SketchView sketch) :
        m_dl_builder(),
        m_dl_denotation_repository_factory(),
        m_dl_denotation_repository(m_dl_denotation_repository_factory.create()),
        m_expander(sketch)
    {
    }

    BoundExecutionContext<Kind> context_at(tyr::planning::StateView<Kind> state, bool is_initial = false)
    {
        return BoundExecutionContext<Kind> { runir::kr::ps::base::EvaluationContext<Kind>(std::move(state), m_dl_builder, m_dl_denotation_repository), is_initial };
    }

    std::optional<RuleView> matching_rule(BoundExecutionContext<Kind>& source, tyr::planning::StateView<Kind> target_state)
    {
        return m_expander.matching_rule(source.context, target_state);
    }
};

}  // namespace

void bind_sketch_executor(nb::module_& m)
{
    auto edge_label = nb::class_<SketchProofEdgeLabel>(m, "SketchProofEdgeLabel")
                          .def_ro("transition", &SketchProofEdgeLabel::transition)
                          .def_ro("rule", &SketchProofEdgeLabel::rule);
    ygg::add_print(edge_label);
    ygg::add_hash(edge_label);

    nb::enum_<SketchProofStatus>(m, "SketchProofStatus")
        .value("SUCCESS", SketchProofStatus::SUCCESS)
        .value("FAILURE", SketchProofStatus::FAILURE)
        .value("OUT_OF_TIME", SketchProofStatus::OUT_OF_TIME)
        .value("OUT_OF_STATES", SketchProofStatus::OUT_OF_STATES);

    bind_sketch_proof_types<tyr::planning::GroundTag>(m, "Ground");
    bind_sketch_proof_types<tyr::planning::LiftedTag>(m, "Lifted");
    bind_sketch_search_options<tyr::planning::GroundTag>(m, "GroundSketchSearchOptions");
    bind_sketch_search_options<tyr::planning::LiftedTag>(m, "LiftedSketchSearchOptions");


    using Kind = tyr::planning::GroundTag;
    using BoundContext = BoundExecutionContext<Kind>;
    using BoundExpander = BoundSuccessorExpander<Kind>;

    nb::class_<BoundContext>(m, "ExecutionContext")
        .def_prop_ro("state", [](const BoundContext& bound) { return bound.context.get_state(); })
        .def_prop_ro("is_initial", [](const BoundContext& bound) { return bound.is_initial; })
        .def("identity_key", &context_key<Kind>);

    nb::class_<BoundExpander>(m, "SuccessorExpander")
        .def(nb::init<SketchView>(), "sketch"_a)
        .def("context_at", &BoundExpander::context_at, "state"_a, "is_initial"_a = false)
        .def("matching_rule", &BoundExpander::matching_rule, "context"_a, "target_state"_a);

    m.def(
        "prove_ground_solution",
        [](runir::datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context,
           SketchView sketch,
           const SketchSearchOptions<tyr::planning::GroundTag>& options) { return prove_solution(std::move(context), sketch, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "sketch"_a,
        "options"_a = SketchSearchOptions<tyr::planning::GroundTag>());
    m.def(
        "prove_lifted_solution",
        [](runir::datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context,
           SketchView sketch,
           const SketchSearchOptions<tyr::planning::LiftedTag>& options) { return prove_solution(std::move(context), sketch, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "sketch"_a,
        "options"_a = SketchSearchOptions<tyr::planning::LiftedTag>());
    m.def(
        "find_ground_solution",
        [](runir::datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context,
           SketchView sketch,
           const SketchSearchOptions<tyr::planning::GroundTag>& options) { return find_solution(std::move(context), sketch, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "sketch"_a,
        "options"_a = SketchSearchOptions<tyr::planning::GroundTag>());
    m.def(
        "find_lifted_solution",
        [](runir::datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context,
           SketchView sketch,
           const SketchSearchOptions<tyr::planning::LiftedTag>& options) { return find_solution(std::move(context), sketch, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "sketch"_a,
        "options"_a = SketchSearchOptions<tyr::planning::LiftedTag>());
}

}  // namespace runir::kr::ps::base
