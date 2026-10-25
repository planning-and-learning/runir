#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_EVALUATION_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/denotation_delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/set_operation_workspace.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <span>
#include <stdexcept>
#include <tyr/planning/declarations.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/database/semantics/incremental/delta.hpp>
#include <yggdrasil/database/semantics/relation_builder.hpp>

namespace runir::kr::dl::semantics::incremental
{

/// Prepared dependency graph shared by any number of feature roots: the interned nodes, which hold their results,
/// in topological order (children precede their consumers), and the resources they share. It has no notion of
/// states; IncrementalEvaluation schedules the nodes.
/// The task and expression repositories must outlive this graph. Indices belong to this graph. Result views borrow
/// it and its task repository; mutation invalidates iterators, and moving the graph invalidates borrowed views.
template<tyr::TaskKind Kind>
class EvaluationGraph
{
    template<tyr::TaskKind>
    friend class IncrementalEvaluation;
    friend class detail::GraphBuilder<Kind>;

    std::vector<std::variant<detail::FeatureNode<Kind, ConceptTag>,
                             detail::FeatureNode<Kind, RoleTag>,
                             detail::FeatureNode<Kind, BooleanTag>,
                             detail::FeatureNode<Kind, NumericalTag>,
                             detail::QueryNode<Kind>>>
        m_nodes;
    detail::SetOperationWorkspace m_set_workspace;
    const tyr::planning::Task<Kind>* m_task;

    template<CategoryTag Category>
    [[nodiscard]] const auto& node(EvaluationIndex<Category> index) const
    { return std::get<detail::FeatureNode<Kind, Category>>(m_nodes.at(static_cast<ygg::uint_t>(index))); }
    [[nodiscard]] const auto& node(QueryEvaluationIndex index) const { return std::get<detail::QueryNode<Kind>>(m_nodes.at(static_cast<ygg::uint_t>(index))); }

    template<CategoryTag Category>
    std::optional<EvaluationIndex<Category>> find_node(ConstructorView<Category> expression) const;
    [[nodiscard]] std::optional<QueryEvaluationIndex> find_node(QueryView<> expression) const;

    template<CategoryTag Category>
    EvaluationIndex<Category> prepare(ConstructorView<Category> expression);
    QueryEvaluationIndex prepare(QueryView<> expression);
    template<CategoryTag Category>
    EvaluationIndex<Category> prepare(ConstructorView<Category> expression, std::vector<ygg::uint_t>& dependencies)
    {
        const auto index = prepare(expression);
        dependencies.push_back(static_cast<ygg::uint_t>(index));
        return index;
    }
    QueryEvaluationIndex prepare(QueryView<> expression, std::vector<ygg::uint_t>& dependencies)
    {
        const auto index = prepare(expression);
        dependencies.push_back(static_cast<ygg::uint_t>(index));
        return index;
    }
    /// The relation of a query, or of a concept or role lifted into the given columns. Identical
    /// lifts share one node.
    QueryEvaluationIndex prepare_relation(QueryView<> expression, std::span<const ygg::database::ColumnLayout>, std::vector<ygg::uint_t>& dependencies)
    { return prepare(expression, dependencies); }
    template<ConceptOrRoleTag Category>
    QueryEvaluationIndex
    prepare_relation(ConstructorView<Category> expression, std::span<const ygg::database::ColumnLayout> columns, std::vector<ygg::uint_t>& dependencies)
    {
        const auto argument = prepare(expression);
        auto index = std::optional<QueryEvaluationIndex> {};
        for (size_t i = 0; i < m_nodes.size() && !index; ++i)
            if (const auto* candidate = std::get_if<detail::QueryNode<Kind>>(&m_nodes[i]); candidate && candidate->lifts(argument, columns))
                index = QueryEvaluationIndex(ygg::to_uint_t(i));
        if (!index)
        {
            index = QueryEvaluationIndex(ygg::to_uint_t(m_nodes.size()));
            m_nodes.emplace_back(detail::QueryNode<Kind>(argument, columns));
        }
        dependencies.push_back(static_cast<ygg::uint_t>(*index));
        return *index;
    }

    /// The node of a Datalog program; one per program.
    QueryEvaluationIndex prepare_program(DatalogProgramView program, std::vector<ygg::uint_t>& dependencies)
    {
        auto index = std::optional<QueryEvaluationIndex> {};
        for (size_t i = 0; i < m_nodes.size() && !index; ++i)
            if (const auto* candidate = std::get_if<detail::QueryNode<Kind>>(&m_nodes[i]);
                candidate && candidate->get_program() && candidate->get_program()->program == program)
                index = QueryEvaluationIndex(ygg::to_uint_t(i));
        if (!index)
        {
            // Construct locally: preparing the inputs may relocate m_nodes.
            auto builder = detail::GraphBuilder<Kind>(*this);
            auto prepared = detail::QueryNode<Kind>(program, builder);
            index = QueryEvaluationIndex(ygg::to_uint_t(m_nodes.size()));
            m_nodes.emplace_back(std::move(prepared));
        }
        dependencies.push_back(static_cast<ygg::uint_t>(*index));
        return *index;
    }
    /// Requests a head of a program node; returns its output.
    size_t request_head(QueryEvaluationIndex program, ygg::Index<ygg::database::datalog::Relation> relation)
    { return std::get<detail::QueryNode<Kind>>(m_nodes.at(static_cast<ygg::uint_t>(program))).get_program()->request(relation); }

public:
    /// The operation of a program node, read by its heads.
    [[nodiscard]] const detail::QueryProgramEvaluator<Kind>& program(QueryEvaluationIndex index) const { return *node(index).get_program(); }

    EvaluationGraph(const tyr::planning::Task<Kind>& task, std::span<const EvaluationRoot> roots);
    EvaluationGraph(const tyr::planning::Task<Kind>& task, std::initializer_list<EvaluationRoot> roots) :
        EvaluationGraph(task, std::span<const EvaluationRoot>(roots.begin(), roots.size()))
    {
    }

    ~EvaluationGraph();
    EvaluationGraph(const EvaluationGraph&) = delete;
    EvaluationGraph& operator=(const EvaluationGraph&) = delete;
    EvaluationGraph(EvaluationGraph&&) noexcept;
    EvaluationGraph& operator=(EvaluationGraph&&) noexcept;

    /// Resolve a prepared root or shared child once; retain its index for evaluation.
    template<CategoryTag Category>
    [[nodiscard]] EvaluationIndex<Category> get_index(ConstructorView<Category> expression) const;
    [[nodiscard]] QueryEvaluationIndex get_index(QueryView<> expression) const;

    [[nodiscard]] size_t node_count() const noexcept;

    /// The results and changes of nodes, read by evaluators from their children (brought to the current state first,
    /// see IncrementalEvaluation).
    template<CategoryTag Category>
    [[nodiscard]] BorrowedDenotationView<Category> result(EvaluationIndex<Category> index) const;
    [[nodiscard]] const ygg::Builder<ygg::database::Relation>& result(QueryEvaluationIndex index) const;
    template<ConceptOrRoleTag Category>
    [[nodiscard]] const DenotationDelta<Category>& change(EvaluationIndex<Category> index) const;
    [[nodiscard]] const ygg::database::incremental::Delta& change(QueryEvaluationIndex index) const;
    template<ConceptOrRoleTag Category>
    [[nodiscard]] size_t cardinality(EvaluationIndex<Category> index) const;
    [[nodiscard]] size_t cardinality(QueryEvaluationIndex index) const { return result(index).size(); }
    template<ConceptOrRoleTag Category>
    [[nodiscard]] bool nonempty(EvaluationIndex<Category> index) const
    { return result(index).any(); }
    [[nodiscard]] bool nonempty(QueryEvaluationIndex index) const { return !result(index).empty(); }
    [[nodiscard]] const tyr::formalism::planning::Repository& repository() const { return *m_task->get_repository(); }
    detail::SetOperationWorkspace& set_workspace() noexcept { return m_set_workspace; }
};

namespace detail
{
/// The build interface of an EvaluationGraph, handed to a node while it is constructed: prepares (or finds) the
/// node's children and records them as its dependencies.
template<tyr::TaskKind Kind>
class GraphBuilder
{
    EvaluationGraph<Kind>* m_graph;

public:
    explicit GraphBuilder(EvaluationGraph<Kind>& graph) noexcept : m_graph(&graph) {}

    template<CategoryTag Category>
    EvaluationIndex<Category> prepare(ConstructorView<Category> expression)
    { return m_graph->prepare(expression); }
    QueryEvaluationIndex prepare(QueryView<> expression) { return m_graph->prepare(expression); }
    template<CategoryTag Category>
    EvaluationIndex<Category> prepare(ConstructorView<Category> expression, std::vector<ygg::uint_t>& dependencies)
    { return m_graph->prepare(expression, dependencies); }
    QueryEvaluationIndex prepare(QueryView<> expression, std::vector<ygg::uint_t>& dependencies) { return m_graph->prepare(expression, dependencies); }
    QueryEvaluationIndex prepare_program(DatalogProgramView program, std::vector<ygg::uint_t>& dependencies)
    { return m_graph->prepare_program(program, dependencies); }
    size_t request_head(QueryEvaluationIndex program, ygg::Index<ygg::database::datalog::Relation> relation)
    { return m_graph->request_head(program, relation); }
    QueryEvaluationIndex prepare_relation(QueryView<> expression, std::span<const ygg::database::ColumnLayout> columns, std::vector<ygg::uint_t>& dependencies)
    { return m_graph->prepare_relation(expression, columns, dependencies); }
    template<ConceptOrRoleTag Category>
    QueryEvaluationIndex
    prepare_relation(ConstructorView<Category> expression, std::span<const ygg::database::ColumnLayout> columns, std::vector<ygg::uint_t>& dependencies)
    { return m_graph->prepare_relation(expression, columns, dependencies); }
};
}  // namespace detail

/// The handle of one state's evaluation, and the only way to read its results: the state's inputs (Context) and the
/// transition into it, which must outlive the handle. A handle is stale once its IncrementalEvaluation entered
/// another state; moving that evaluation invalidates it.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context>
class StateEvaluation;

/// Demand-driven evaluation of an EvaluationGraph over a sequence of states. Each node records the epoch (entered
/// state) it was last evaluated in, a verifying trace as in build systems (Mokhov, Mitchell & Peyton Jones, "Build
/// Systems à la Carte", ICFP 2018), and nodes are brought up to date only when demanded, as in Adapton (Hammer et al.,
/// "Adapton: Composable, Demand-Driven Incremental Computation", PLDI 2014). Demanding a node first brings its children
/// up to date. A node evaluated in the previous epoch whose children all hold the exact delta of this transition is
/// updated from that delta (the evaluators' delta rules); otherwise it is initialized from the state's inputs. Nodes
/// not demanded for two or more epochs are thus initialized again rather than replaying the skipped deltas. A node
/// whose evaluation throws is initialized on its next demand. Whether a node was updated from the delta is observable:
/// StateEvaluation::delta() and changed() throw otherwise.
template<tyr::TaskKind Kind>
class IncrementalEvaluation
{
    template<tyr::TaskKind K, StateEvaluationContextConcept<K> C>
    friend class StateEvaluation;

    struct NodeState
    {
        uint64_t epoch = 0;      ///< The epoch the node was last evaluated in; 0 if never, or its last evaluation failed.
        bool has_delta = false;  ///< Updated from the transition delta, so its delta is exact.
    };

    EvaluationGraph<Kind> m_graph;
    std::vector<NodeState> m_nodes;
    uint64_t m_epoch = 0;
    bool m_transition = false;  ///< m_epoch was entered from the previous one by a delta.

    template<StateEvaluationContextConcept<Kind> Context>
    void require_task(const Context& context) const
    {
        if (&context.get_state().get_task() != m_graph.m_task)
            throw std::invalid_argument("Incremental evaluation: graph belongs to a different task.");
    }
    void require_exact(ygg::uint_t index) const
    {
        if (!m_nodes[index].has_delta)
            throw std::logic_error("Incremental evaluation: exact deltas exist only for results updated from the state's delta.");
    }
    template<StateEvaluationContextConcept<Kind> Context>
    StateEvaluation<Kind, Context> next(Context context, const Delta* delta, ygg::database::Workspace* workspace)
    {
        ++m_epoch;
        m_transition = delta != nullptr;
        return StateEvaluation<Kind, Context>(*this, std::move(context), delta, workspace);
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void demand(ygg::uint_t index, Context& context, const Delta* delta, ygg::database::Workspace* workspace);

public:
    IncrementalEvaluation(const tyr::planning::Task<Kind>& task, std::span<const EvaluationRoot> roots) : m_graph(task, roots), m_nodes(m_graph.node_count()) {}
    IncrementalEvaluation(const tyr::planning::Task<Kind>& task, std::initializer_list<EvaluationRoot> roots) :
        IncrementalEvaluation(task, std::span<const EvaluationRoot>(roots.begin(), roots.size()))
    {
    }
    ~IncrementalEvaluation();
    IncrementalEvaluation(IncrementalEvaluation&&) = default;
    IncrementalEvaluation& operator=(IncrementalEvaluation&&) = default;
    IncrementalEvaluation(const IncrementalEvaluation&) = delete;
    IncrementalEvaluation& operator=(const IncrementalEvaluation&) = delete;

    [[nodiscard]] const EvaluationGraph<Kind>& graph() const noexcept { return m_graph; }

    /// Enters a state unrelated to the previous one: every demanded node is initialized.
    template<StateEvaluationContextConcept<Kind> Context>
    StateEvaluation<Kind, Context> enter(Context context)
    {
        require_task(context);
        return next(std::move(context), nullptr, nullptr);
    }
    /// Enters the state reached from the previous one by `delta`.
    template<StateEvaluationContextConcept<Kind> Context>
    StateEvaluation<Kind, Context> enter(Context context, const Delta& delta, ygg::database::Workspace& workspace)
    {
        require_task(context);
        return next(std::move(context), &delta, &workspace);
    }
    /// Continues the current state, keeping what it evaluated. `delta` must be the one the state was entered by; it
    /// is ignored if the state was entered without one.
    template<StateEvaluationContextConcept<Kind> Context>
    StateEvaluation<Kind, Context> resume(Context context, const Delta& delta, ygg::database::Workspace& workspace)
    {
        require_task(context);
        if (m_epoch == 0)
            throw std::logic_error("Incremental evaluation: enter a state before resuming it.");
        return StateEvaluation<Kind, Context>(*this, std::move(context), m_transition ? &delta : nullptr, &workspace);
    }

private:
    /// Results of nodes evaluated in the current state, read through StateEvaluation, which demands them first.
    template<CategoryTag Category>
    [[nodiscard]] BorrowedDenotationView<Category> get_result(EvaluationIndex<Category> index) const
    { return m_graph.result(index); }
    [[nodiscard]] const ygg::Builder<ygg::database::Relation>& get_result(QueryEvaluationIndex index) const { return m_graph.result(index); }
    template<ConceptOrRoleTag Category>
    [[nodiscard]] size_t size(EvaluationIndex<Category> index) const
    { return m_graph.cardinality(index); }
    [[nodiscard]] size_t size(QueryEvaluationIndex index) const { return m_graph.cardinality(index); }
    /// Changes of nodes updated from the transition into the current state.
    template<ConceptOrRoleTag Category>
    [[nodiscard]] const DenotationDelta<Category>& get_delta(EvaluationIndex<Category> index) const
    {
        require_exact(static_cast<ygg::uint_t>(index));
        return m_graph.change(index);
    }
    [[nodiscard]] const ygg::database::incremental::Delta& get_delta(QueryEvaluationIndex index) const
    {
        require_exact(static_cast<ygg::uint_t>(index));
        return m_graph.change(index);
    }
    template<BooleanOrNumericalTag Category>
    [[nodiscard]] bool changed(EvaluationIndex<Category> index) const;
};

template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context>
class StateEvaluation
{
    IncrementalEvaluation<Kind>* m_evaluation;
    uint64_t m_epoch;
    Context m_context;
    const Delta* m_delta;  ///< The transition into the state, if known.
    ygg::database::Workspace* m_workspace;

    friend class IncrementalEvaluation<Kind>;
    StateEvaluation(IncrementalEvaluation<Kind>& evaluation, Context context, const Delta* delta, ygg::database::Workspace* workspace) :
        m_evaluation(&evaluation),
        m_epoch(evaluation.m_epoch),
        m_context(std::move(context)),
        m_delta(delta),
        m_workspace(workspace)
    {
    }
    IncrementalEvaluation<Kind>& current(ygg::uint_t index)
    {
        if (m_epoch != m_evaluation->m_epoch)
            throw std::logic_error("Incremental evaluation: this state evaluation is stale.");
        m_evaluation->demand(index, m_context, m_delta, m_workspace);
        return *m_evaluation;
    }

public:
    [[nodiscard]] const EvaluationGraph<Kind>& graph() const noexcept { return m_evaluation->graph(); }
    [[nodiscard]] const Context& context() const noexcept { return m_context; }

    template<CategoryTag Category>
    BorrowedDenotationView<Category> evaluate(EvaluationIndex<Category> index)
    { return current(static_cast<ygg::uint_t>(index)).get_result(index); }
    const ygg::Builder<ygg::database::Relation>& evaluate(QueryEvaluationIndex index) { return current(static_cast<ygg::uint_t>(index)).get_result(index); }
    template<ConceptOrRoleTag Category>
    size_t size(EvaluationIndex<Category> index)
    { return current(static_cast<ygg::uint_t>(index)).size(index); }
    size_t size(QueryEvaluationIndex index) { return current(static_cast<ygg::uint_t>(index)).size(index); }
    template<ConceptOrRoleTag Category>
    const DenotationDelta<Category>& delta(EvaluationIndex<Category> index)
    { return current(static_cast<ygg::uint_t>(index)).get_delta(index); }
    const ygg::database::incremental::Delta& delta(QueryEvaluationIndex index) { return current(static_cast<ygg::uint_t>(index)).get_delta(index); }
    template<BooleanOrNumericalTag Category>
    bool changed(EvaluationIndex<Category> index)
    { return current(static_cast<ygg::uint_t>(index)).changed(index); }
};

}  // namespace runir::kr::dl::semantics::incremental

#ifdef RUNIR_HEADER_INSTANTIATION
#include "runir/kr/dl/semantics/incremental/detail/evaluation_impl.hpp"
#endif

#endif
