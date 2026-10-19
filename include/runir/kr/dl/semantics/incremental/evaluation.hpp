#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_EVALUATION_HPP_

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
#include <utility>
#include <variant>
#include <vector>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/database/semantics/incremental/delta.hpp>
#include <yggdrasil/database/semantics/relation_builder.hpp>

namespace runir::kr::dl::semantics::incremental
{

/// Prepared dependency graph shared by any number of feature roots: the interned nodes, which hold their results,
/// in topological order (children precede their consumers), and the resources they share. It has no notion of
/// states; IncrementalEvaluation schedules the nodes.
/// The task and expression repositories must outlive this graph. Indices belong to this graph. Result views borrow
/// it and its task repository; mutation invalidates iterators, and moving the graph invalidates borrowed views.
template<FamilyTag Family, tyr::TaskKind Kind>
class EvaluationGraph
{
    template<FamilyTag, tyr::TaskKind>
    friend class IncrementalEvaluation;
    friend class detail::GraphBuilder<Family, Kind>;

    std::vector<std::variant<detail::FeatureNode<Family, Kind, ConceptTag>,
                             detail::FeatureNode<Family, Kind, RoleTag>,
                             detail::FeatureNode<Family, Kind, BooleanTag>,
                             detail::FeatureNode<Family, Kind, NumericalTag>,
                             detail::QueryNode<Family, Kind>>>
        m_nodes;
    detail::SetOperationWorkspace m_set_workspace;
    const tyr::planning::Task<Kind>* m_task;

    template<CategoryTag Category>
    const auto& node(EvaluationIndex<Category> index) const
    {
        return std::get<detail::FeatureNode<Family, Kind, Category>>(m_nodes.at(ygg::uint_t(index)));
    }
    const auto& node(QueryEvaluationIndex index) const { return std::get<detail::QueryNode<Family, Kind>>(m_nodes.at(ygg::uint_t(index))); }

    template<CategoryTag Category>
    std::optional<EvaluationIndex<Category>> find_node(FamilyConstructorView<Family, Category> expression) const;
    std::optional<QueryEvaluationIndex> find_node(FamilyQueryView<Family> expression) const;

    template<CategoryTag Category>
    EvaluationIndex<Category> prepare(FamilyConstructorView<Family, Category> expression);
    QueryEvaluationIndex prepare(FamilyQueryView<Family> expression);
    template<CategoryTag Category>
    EvaluationIndex<Category> prepare(FamilyConstructorView<Family, Category> expression, std::vector<ygg::uint_t>& dependencies)
    {
        const auto index = prepare(expression);
        dependencies.push_back(ygg::uint_t(index));
        return index;
    }
    QueryEvaluationIndex prepare(FamilyQueryView<Family> expression, std::vector<ygg::uint_t>& dependencies)
    {
        const auto index = prepare(expression);
        dependencies.push_back(ygg::uint_t(index));
        return index;
    }
    /// The relation of a query, or of a concept or role lifted into the given columns. Identical
    /// lifts share one node.
    QueryEvaluationIndex
    prepare_relation(FamilyQueryView<Family> expression, std::span<const ygg::database::ColumnLayout>, std::vector<ygg::uint_t>& dependencies)
    {
        return prepare(expression, dependencies);
    }
    template<ConceptOrRoleTag Category>
    QueryEvaluationIndex prepare_relation(FamilyConstructorView<Family, Category> expression,
                                          std::span<const ygg::database::ColumnLayout> columns,
                                          std::vector<ygg::uint_t>& dependencies)
    {
        const auto argument = prepare(expression);
        auto index = std::optional<QueryEvaluationIndex> {};
        for (size_t i = 0; i < m_nodes.size() && !index; ++i)
            if (const auto* candidate = std::get_if<detail::QueryNode<Family, Kind>>(&m_nodes[i]); candidate && candidate->lifts(argument, columns))
                index = QueryEvaluationIndex(ygg::to_uint_t(i));
        if (!index)
        {
            index = QueryEvaluationIndex(ygg::to_uint_t(m_nodes.size()));
            m_nodes.emplace_back(detail::QueryNode<Family, Kind>(argument, columns));
        }
        dependencies.push_back(ygg::uint_t(*index));
        return *index;
    }

public:
    EvaluationGraph(const tyr::planning::Task<Kind>& task, std::span<const EvaluationRoot<Family>> roots);
    EvaluationGraph(const tyr::planning::Task<Kind>& task, std::initializer_list<EvaluationRoot<Family>> roots) :
        EvaluationGraph(task, std::span<const EvaluationRoot<Family>>(roots.begin(), roots.size()))
    {
    }

    ~EvaluationGraph();
    EvaluationGraph(const EvaluationGraph&) = delete;
    EvaluationGraph& operator=(const EvaluationGraph&) = delete;
    EvaluationGraph(EvaluationGraph&&) noexcept;
    EvaluationGraph& operator=(EvaluationGraph&&) noexcept;

    /// Resolve a prepared root or shared child once; retain its index for evaluation.
    template<CategoryTag Category>
    EvaluationIndex<Category> get_index(FamilyConstructorView<Family, Category> expression) const;
    QueryEvaluationIndex get_index(FamilyQueryView<Family> expression) const;

    size_t node_count() const noexcept;

    /// The results and changes of nodes, read by evaluators from their children (brought to the current state first,
    /// see IncrementalEvaluation).
    template<CategoryTag Category>
    BorrowedDenotationView<Category> result(EvaluationIndex<Category> index) const;
    const ygg::Builder<ygg::database::Relation<QueryValues>>& result(QueryEvaluationIndex index) const;
    template<ConceptOrRoleTag Category>
    const DenotationDelta<Category>& change(EvaluationIndex<Category> index) const;
    const ygg::database::incremental::Delta<QueryValues>& change(QueryEvaluationIndex index) const;
    template<ConceptOrRoleTag Category>
    size_t cardinality(EvaluationIndex<Category> index) const;
    size_t cardinality(QueryEvaluationIndex index) const { return result(index).size(); }
    template<ConceptOrRoleTag Category>
    bool nonempty(EvaluationIndex<Category> index) const
    {
        return result(index).any();
    }
    bool nonempty(QueryEvaluationIndex index) const { return !result(index).empty(); }
    const tyr::formalism::planning::Repository& repository() const { return *m_task->get_repository(); }
    detail::SetOperationWorkspace& set_workspace() noexcept { return m_set_workspace; }
};

namespace detail
{
/// The build interface of an EvaluationGraph, handed to a node while it is constructed: prepares (or finds) the
/// node's children and records them as its dependencies.
template<FamilyTag Family, tyr::TaskKind Kind>
class GraphBuilder
{
    EvaluationGraph<Family, Kind>* m_graph;

public:
    explicit GraphBuilder(EvaluationGraph<Family, Kind>& graph) noexcept : m_graph(&graph) {}

    template<CategoryTag Category>
    EvaluationIndex<Category> prepare(FamilyConstructorView<Family, Category> expression)
    {
        return m_graph->prepare(expression);
    }
    QueryEvaluationIndex prepare(FamilyQueryView<Family> expression) { return m_graph->prepare(expression); }
    template<CategoryTag Category>
    EvaluationIndex<Category> prepare(FamilyConstructorView<Family, Category> expression, std::vector<ygg::uint_t>& dependencies)
    {
        return m_graph->prepare(expression, dependencies);
    }
    QueryEvaluationIndex prepare(FamilyQueryView<Family> expression, std::vector<ygg::uint_t>& dependencies)
    {
        return m_graph->prepare(expression, dependencies);
    }
    QueryEvaluationIndex
    prepare_relation(FamilyQueryView<Family> expression, std::span<const ygg::database::ColumnLayout> columns, std::vector<ygg::uint_t>& dependencies)
    {
        return m_graph->prepare_relation(expression, columns, dependencies);
    }
    template<ConceptOrRoleTag Category>
    QueryEvaluationIndex prepare_relation(FamilyConstructorView<Family, Category> expression,
                                          std::span<const ygg::database::ColumnLayout> columns,
                                          std::vector<ygg::uint_t>& dependencies)
    {
        return m_graph->prepare_relation(expression, columns, dependencies);
    }
};
}  // namespace detail

/// The handle of one state's evaluation, and the only way to read its results: the state's inputs (Context) and the
/// transition into it, which must outlive the handle. A handle is stale once its IncrementalEvaluation entered
/// another state; moving that evaluation invalidates it.
template<FamilyTag Family, tyr::TaskKind Kind, StateEvaluationContextConcept<Family, Kind> Context>
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
template<FamilyTag Family, tyr::TaskKind Kind>
class IncrementalEvaluation
{
    template<FamilyTag F, tyr::TaskKind K, StateEvaluationContextConcept<F, K> C>
    friend class StateEvaluation;

    struct NodeState
    {
        uint64_t epoch = 0;      ///< The epoch the node was last evaluated in; 0 if never, or its last evaluation failed.
        bool has_delta = false;  ///< Updated from the transition delta, so its delta is exact.
    };

    EvaluationGraph<Family, Kind> m_graph;
    std::vector<NodeState> m_nodes;
    uint64_t m_epoch = 0;
    bool m_transition = false;  ///< m_epoch was entered from the previous one by a delta.

    template<StateEvaluationContextConcept<Family, Kind> Context>
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
    template<StateEvaluationContextConcept<Family, Kind> Context>
    StateEvaluation<Family, Kind, Context> next(Context context, const Delta<Family>* delta, ygg::database::Workspace<QueryValues>* workspace)
    {
        ++m_epoch;
        m_transition = delta != nullptr;
        return StateEvaluation<Family, Kind, Context>(*this, std::move(context), delta, workspace);
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void demand(ygg::uint_t index, Context& context, const Delta<Family>* delta, ygg::database::Workspace<QueryValues>* workspace);

public:
    IncrementalEvaluation(const tyr::planning::Task<Kind>& task, std::span<const EvaluationRoot<Family>> roots) :
        m_graph(task, roots),
        m_nodes(m_graph.node_count())
    {
    }
    IncrementalEvaluation(const tyr::planning::Task<Kind>& task, std::initializer_list<EvaluationRoot<Family>> roots) :
        IncrementalEvaluation(task, std::span<const EvaluationRoot<Family>>(roots.begin(), roots.size()))
    {
    }
    ~IncrementalEvaluation();
    IncrementalEvaluation(IncrementalEvaluation&&) = default;
    IncrementalEvaluation& operator=(IncrementalEvaluation&&) = default;

    const EvaluationGraph<Family, Kind>& graph() const noexcept { return m_graph; }

    /// Enters a state unrelated to the previous one: every demanded node is initialized.
    template<StateEvaluationContextConcept<Family, Kind> Context>
    StateEvaluation<Family, Kind, Context> enter(Context context)
    {
        require_task(context);
        return next(std::move(context), nullptr, nullptr);
    }
    /// Enters the state reached from the previous one by `delta`.
    template<StateEvaluationContextConcept<Family, Kind> Context>
    StateEvaluation<Family, Kind, Context> enter(Context context, const Delta<Family>& delta, ygg::database::Workspace<QueryValues>& workspace)
    {
        require_task(context);
        return next(std::move(context), &delta, &workspace);
    }
    /// Continues the current state, keeping what it evaluated. `delta` must be the one the state was entered by; it
    /// is ignored if the state was entered without one.
    template<StateEvaluationContextConcept<Family, Kind> Context>
    StateEvaluation<Family, Kind, Context> resume(Context context, const Delta<Family>& delta, ygg::database::Workspace<QueryValues>& workspace)
    {
        require_task(context);
        if (m_epoch == 0)
            throw std::logic_error("Incremental evaluation: enter a state before resuming it.");
        return StateEvaluation<Family, Kind, Context>(*this, std::move(context), m_transition ? &delta : nullptr, &workspace);
    }

private:
    /// Results of nodes evaluated in the current state, read through StateEvaluation, which demands them first.
    template<CategoryTag Category>
    BorrowedDenotationView<Category> get_result(EvaluationIndex<Category> index) const
    {
        return m_graph.result(index);
    }
    const ygg::Builder<ygg::database::Relation<QueryValues>>& get_result(QueryEvaluationIndex index) const { return m_graph.result(index); }
    template<ConceptOrRoleTag Category>
    size_t size(EvaluationIndex<Category> index) const
    {
        return m_graph.cardinality(index);
    }
    size_t size(QueryEvaluationIndex index) const { return m_graph.cardinality(index); }
    /// Changes of nodes updated from the transition into the current state.
    template<ConceptOrRoleTag Category>
    const DenotationDelta<Category>& get_delta(EvaluationIndex<Category> index) const
    {
        require_exact(ygg::uint_t(index));
        return m_graph.change(index);
    }
    const ygg::database::incremental::Delta<QueryValues>& get_delta(QueryEvaluationIndex index) const
    {
        require_exact(ygg::uint_t(index));
        return m_graph.change(index);
    }
    template<BooleanOrNumericalTag Category>
    bool changed(EvaluationIndex<Category> index) const;
};

template<FamilyTag Family, tyr::TaskKind Kind, StateEvaluationContextConcept<Family, Kind> Context>
class StateEvaluation
{
    IncrementalEvaluation<Family, Kind>* m_evaluation;
    uint64_t m_epoch;
    Context m_context;
    const Delta<Family>* m_delta;  ///< The transition into the state, if known.
    ygg::database::Workspace<QueryValues>* m_workspace;

    friend class IncrementalEvaluation<Family, Kind>;
    StateEvaluation(IncrementalEvaluation<Family, Kind>& evaluation,
                    Context context,
                    const Delta<Family>* delta,
                    ygg::database::Workspace<QueryValues>* workspace) :
        m_evaluation(&evaluation),
        m_epoch(evaluation.m_epoch),
        m_context(std::move(context)),
        m_delta(delta),
        m_workspace(workspace)
    {
    }
    IncrementalEvaluation<Family, Kind>& current(ygg::uint_t index)
    {
        if (m_epoch != m_evaluation->m_epoch)
            throw std::logic_error("Incremental evaluation: this state evaluation is stale.");
        m_evaluation->demand(index, m_context, m_delta, m_workspace);
        return *m_evaluation;
    }

public:
    const EvaluationGraph<Family, Kind>& graph() const noexcept { return m_evaluation->graph(); }
    const Context& context() const noexcept { return m_context; }

    template<CategoryTag Category>
    BorrowedDenotationView<Category> evaluate(EvaluationIndex<Category> index)
    {
        return current(ygg::uint_t(index)).get_result(index);
    }
    const ygg::Builder<ygg::database::Relation<QueryValues>>& evaluate(QueryEvaluationIndex index)
    {
        return current(ygg::uint_t(index)).get_result(index);
    }
    template<ConceptOrRoleTag Category>
    size_t size(EvaluationIndex<Category> index)
    {
        return current(ygg::uint_t(index)).size(index);
    }
    size_t size(QueryEvaluationIndex index) { return current(ygg::uint_t(index)).size(index); }
    template<ConceptOrRoleTag Category>
    const DenotationDelta<Category>& delta(EvaluationIndex<Category> index)
    {
        return current(ygg::uint_t(index)).get_delta(index);
    }
    const ygg::database::incremental::Delta<QueryValues>& delta(QueryEvaluationIndex index)
    {
        return current(ygg::uint_t(index)).get_delta(index);
    }
    template<BooleanOrNumericalTag Category>
    bool changed(EvaluationIndex<Category> index)
    {
        return current(ygg::uint_t(index)).changed(index);
    }
};

}  // namespace runir::kr::dl::semantics::incremental

#ifdef RUNIR_HEADER_INSTANTIATION
#include "runir/kr/dl/semantics/incremental/detail/evaluation_impl.hpp"
#endif

#endif
