#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_EVALUATION_HPP_

#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/detail/feature_node.hpp"
#include "runir/kr/dl/semantics/incremental/detail/query_node.hpp"

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
    template<FamilyTag, tyr::TaskKind, CategoryTag>
    friend class detail::FeatureNode;
    template<FamilyTag, tyr::TaskKind>
    friend class detail::QueryNode;

    template<FamilyTag, tyr::TaskKind, CategoryTag, tyr::formalism::FactKind>
    friend struct detail::AtomicEvaluator;
    template<FamilyTag, tyr::TaskKind, CategoryTag, typename, ConceptOrRoleTag>
    friend struct detail::UnarySetEvaluator;
    template<FamilyTag, tyr::TaskKind, CategoryTag, typename, ConceptOrRoleTag, ConceptOrRoleTag>
    friend struct detail::BinarySetEvaluator;
    template<FamilyTag, tyr::TaskKind, typename>
    friend struct detail::NumberRestrictionEvaluator;
    template<FamilyTag, tyr::TaskKind, typename>
    friend struct detail::QualifiedNumberRestrictionEvaluator;
    template<FamilyTag, tyr::TaskKind>
    friend struct detail::FillersEvaluator;
    template<FamilyTag, tyr::TaskKind, CategoryTag, typename, BooleanOrNumericalTag>
    friend struct detail::ScalarBinaryEvaluator;
    template<FamilyTag, tyr::TaskKind>
    friend struct detail::LogicalNotEvaluator;
    template<FamilyTag, tyr::TaskKind>
    friend struct detail::CountEvaluator;
    template<FamilyTag, tyr::TaskKind>
    friend struct detail::NonemptyEvaluator;
    template<FamilyTag, tyr::TaskKind>
    friend struct detail::DistanceFeatureEvaluator;
    template<FamilyTag, tyr::TaskKind>
    friend struct detail::GuidedDistanceEvaluator;
    template<FamilyTag, tyr::TaskKind, CategoryTag>
    friend struct detail::ProjectionEvaluator;
    template<FamilyTag, tyr::TaskKind>
    friend struct detail::QueryProjectionEvaluator;
    template<FamilyTag, tyr::TaskKind>
    friend struct detail::QueryJoinEvaluator;
    template<FamilyTag, tyr::TaskKind>
    friend struct detail::QueryGenericJoinEvaluator;
    template<FamilyTag, tyr::TaskKind, ConceptOrRoleTag>
    friend struct detail::QueryFromDenotationEvaluator;
    template<FamilyTag, tyr::TaskKind, typename>
    friend struct detail::QuerySetCombinationEvaluator;
    template<FamilyTag, tyr::TaskKind, typename>
    friend struct detail::QuerySelectionEvaluator;

    std::vector<std::variant<detail::FeatureNode<Family, Kind, ConceptTag>,
                             detail::FeatureNode<Family, Kind, RoleTag>,
                             detail::FeatureNode<Family, Kind, BooleanTag>,
                             detail::FeatureNode<Family, Kind, NumericalTag>,
                             detail::QueryNode<Family, Kind>>>
        m_nodes;
    detail::SetOperationWorkspace m_set_workspace;
    const tyr::planning::Task<Kind>& m_task;

    template<CategoryTag Category>
    const auto& node(EvaluationIndex<Category> index) const
    {
        return std::get<detail::FeatureNode<Family, Kind, Category>>(m_nodes.at(ygg::uint_t(index)));
    }
    const auto& node(QueryEvaluationIndex index) const { return std::get<detail::QueryNode<Family, Kind>>(m_nodes.at(ygg::uint_t(index))); }

    template<CategoryTag Category>
    std::optional<EvaluationIndex<Category>> find_node(FamilyConstructorView<Family, Category> expression) const;
    std::optional<QueryEvaluationIndex> find_node(FamilyQueryView<Family> expression) const;

    // Evaluators read their children, which are brought to the current state first.
    template<CategoryTag Category>
    BorrowedDenotationView<Category> result(EvaluationIndex<Category> index) const
    {
        return node(index).get_result(repository());
    }
    const auto& result(QueryEvaluationIndex index) const { return node(index).get_result(); }
    template<ConceptOrRoleTag Category>
    const DenotationDelta<Category>& change(EvaluationIndex<Category> index) const
    {
        return node(index).get_delta();
    }
    const auto& change(QueryEvaluationIndex index) const { return node(index).get_delta(); }
    template<ConceptOrRoleTag Category>
    size_t cardinality(EvaluationIndex<Category> index) const
    {
        return node(index).size();
    }
    size_t cardinality(QueryEvaluationIndex index) const { return result(index).size(); }
    template<ConceptOrRoleTag Category>
    bool nonempty(EvaluationIndex<Category> index) const
    {
        return result(index).any();
    }
    bool nonempty(QueryEvaluationIndex index) const { return !result(index).empty(); }
    const tyr::formalism::planning::Repository& repository() const { return *m_task.get_repository(); }
    detail::SetOperationWorkspace& set_workspace() noexcept { return m_set_workspace; }

    template<CategoryTag Category>
    EvaluationIndex<Category> prepare(FamilyConstructorView<Family, Category> expression);
    QueryEvaluationIndex prepare(FamilyQueryView<Family> expression);
    template<typename Expression>
    auto prepare(Expression expression, std::vector<ygg::uint_t>& dependencies)
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

    EvaluationGraph(const EvaluationGraph&) = delete;
    EvaluationGraph& operator=(const EvaluationGraph&) = delete;
    EvaluationGraph(EvaluationGraph&&) = default;

    /// Resolve a prepared root or shared child once; retain its index for evaluation.
    template<CategoryTag Category>
    EvaluationIndex<Category> get_index(FamilyConstructorView<Family, Category> expression) const;
    QueryEvaluationIndex get_index(FamilyQueryView<Family> expression) const;

    size_t node_count() const noexcept { return m_nodes.size(); }
};

template<FamilyTag Family, tyr::TaskKind Kind>
EvaluationGraph<Family, Kind>::EvaluationGraph(const tyr::planning::Task<Kind>& task, std::span<const EvaluationRoot<Family>> roots) : m_task(task)
{
    m_set_workspace.initialize(ygg::to_uint_t(task.get_task().get_num_objects()));
    for (const auto& root : roots)
        std::visit([&](auto expression) { prepare(expression); }, root);
}

template<FamilyTag Family, tyr::TaskKind Kind>
template<CategoryTag Category>
EvaluationIndex<Category> EvaluationGraph<Family, Kind>::get_index(FamilyConstructorView<Family, Category> expression) const
{
    if (const auto index = find_node(expression))
        return *index;
    throw std::invalid_argument("Incremental evaluation: expression is not prepared in this graph.");
}

template<FamilyTag Family, tyr::TaskKind Kind>
QueryEvaluationIndex EvaluationGraph<Family, Kind>::get_index(FamilyQueryView<Family> expression) const
{
    if (const auto index = find_node(expression))
        return *index;
    throw std::invalid_argument("Incremental evaluation: expression is not prepared in this graph.");
}

template<FamilyTag Family, tyr::TaskKind Kind>
template<CategoryTag Category>
std::optional<EvaluationIndex<Category>> EvaluationGraph<Family, Kind>::find_node(FamilyConstructorView<Family, Category> expression) const
{
    // ponytail: linear preparation/root lookup; add a lookup table only if large graphs make it costly.
    for (size_t i = 0; i < m_nodes.size(); ++i)
    {
        const auto* candidate = std::get_if<detail::FeatureNode<Family, Kind, Category>>(&m_nodes[i]);
        if (candidate && candidate->get_expression() == expression)
            return EvaluationIndex<Category>(ygg::to_uint_t(i));
    }
    return std::nullopt;
}

template<FamilyTag Family, tyr::TaskKind Kind>
std::optional<QueryEvaluationIndex> EvaluationGraph<Family, Kind>::find_node(FamilyQueryView<Family> expression) const
{
    for (size_t i = 0; i < m_nodes.size(); ++i)
    {
        const auto* candidate = std::get_if<detail::QueryNode<Family, Kind>>(&m_nodes[i]);
        if (candidate && candidate->get_expression() == expression)
            return QueryEvaluationIndex(ygg::to_uint_t(i));
    }
    return std::nullopt;
}

template<FamilyTag Family, tyr::TaskKind Kind>
template<CategoryTag Category>
EvaluationIndex<Category> EvaluationGraph<Family, Kind>::prepare(FamilyConstructorView<Family, Category> expression)
{
    if (const auto existing = find_node(expression))
        return *existing;
    // Construct locally: preparing children may relocate m_nodes.
    auto prepared = detail::FeatureNode<Family, Kind, Category>(expression, *this);
    const auto index = EvaluationIndex<Category>(ygg::to_uint_t(m_nodes.size()));
    m_nodes.emplace_back(std::move(prepared));
    return index;
}

template<FamilyTag Family, tyr::TaskKind Kind>
QueryEvaluationIndex EvaluationGraph<Family, Kind>::prepare(FamilyQueryView<Family> expression)
{
    if (const auto existing = find_node(expression))
        return *existing;
    auto prepared = detail::QueryNode<Family, Kind>(expression, *this);
    const auto index = QueryEvaluationIndex(ygg::to_uint_t(m_nodes.size()));
    m_nodes.emplace_back(std::move(prepared));
    return index;
}

/// The handle of one state's evaluation: the state's inputs (Context, or NoContext for an update without them) and
/// the transition into it, which must outlive the handle. A handle is stale once its IncrementalEvaluation entered
/// another state; moving that evaluation invalidates it.
template<FamilyTag Family, tyr::TaskKind Kind, typename Context>
class StateEvaluation;

/// Inputs of an update that may only apply deltas.
struct NoContext
{
};

/// Demand-driven, change-propagating evaluation of an EvaluationGraph over a sequence of states, as in adaptive
/// computation (Acar, "Self-Adjusting Computation", PhD thesis, CMU 2005) and demanded computation graphs (Hammer
/// et al., "Adapton: Composable, Demand-Driven Incremental Computation", PLDI 2014): each node records the state it
/// was last evaluated in. Demanding a node in a state first brings its children up to date. A node evaluated in the
/// previous state whose children all hold the exact delta of this transition is updated from that delta; otherwise
/// it is initialized from the state's inputs. Nodes not demanded for two or more states are thus initialized again
/// rather than replaying the skipped deltas. A node whose evaluation throws is initialized on its next demand.
template<FamilyTag Family, tyr::TaskKind Kind>
class IncrementalEvaluation
{
    template<FamilyTag, tyr::TaskKind, typename>
    friend class StateEvaluation;

    struct NodeState
    {
        uint64_t state = 0;  ///< The state the node was last evaluated in; 0 if never, or its last evaluation failed.
        bool exact = false;  ///< Updated from the transition delta, so its delta is exact.
    };

    EvaluationGraph<Family, Kind> m_graph;
    std::vector<NodeState> m_nodes;
    uint64_t m_state = 0;
    bool m_transition = false;  ///< m_state was entered from the previous state by a delta.

    template<typename Context>
    void require_task(const Context& context) const
    {
        if (&context.get_state().get_task() != &m_graph.m_task)
            throw std::invalid_argument("Incremental evaluation: graph belongs to a different task.");
    }
    void require_current(ygg::uint_t index) const
    {
        if (m_state == 0 || m_nodes.at(index).state != m_state)
            throw std::logic_error("Incremental evaluation: evaluate this result in the current state before reading it.");
    }
    void require_exact(ygg::uint_t index) const
    {
        require_current(index);
        if (!m_nodes[index].exact)
            throw std::logic_error("Incremental evaluation: exact deltas exist only for results updated from the state's delta.");
    }
    template<typename Context>
    StateEvaluation<Family, Kind, Context> next(Context context, const Delta<Family>* delta, ygg::database::Workspace<QueryValues>* workspace)
    {
        ++m_state;
        m_transition = delta != nullptr;
        return StateEvaluation<Family, Kind, Context>(*this, std::move(context), delta, workspace);
    }
    template<typename Context>
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
    /// Enters the state reached by `delta` without its inputs; demanding a node that must be initialized throws.
    StateEvaluation<Family, Kind, NoContext> enter(const Delta<Family>& delta, ygg::database::Workspace<QueryValues>& workspace)
    {
        return next(NoContext {}, &delta, &workspace);
    }
    /// Continues the current state, keeping what it evaluated. `delta` must be the one the state was entered by; it
    /// is ignored if the state was entered without one.
    template<StateEvaluationContextConcept<Family, Kind> Context>
    StateEvaluation<Family, Kind, Context> resume(Context context, const Delta<Family>& delta, ygg::database::Workspace<QueryValues>& workspace)
    {
        require_task(context);
        if (m_state == 0)
            throw std::logic_error("Incremental evaluation: enter a state before resuming it.");
        return StateEvaluation<Family, Kind, Context>(*this, std::move(context), m_transition ? &delta : nullptr, &workspace);
    }

    /// Results of nodes evaluated in the current state.
    template<CategoryTag Category>
    BorrowedDenotationView<Category> get_result(EvaluationIndex<Category> index) const&
    {
        require_current(ygg::uint_t(index));
        return m_graph.result(index);
    }
    template<CategoryTag Category>
    BorrowedDenotationView<Category> get_result(EvaluationIndex<Category>) const&& = delete;
    const auto& get_result(QueryEvaluationIndex index) const&
    {
        require_current(ygg::uint_t(index));
        return m_graph.result(index);
    }
    const auto& get_result(QueryEvaluationIndex) const&& = delete;
    template<ConceptOrRoleTag Category>
    size_t size(EvaluationIndex<Category> index) const
    {
        require_current(ygg::uint_t(index));
        return m_graph.cardinality(index);
    }
    size_t size(QueryEvaluationIndex index) const
    {
        require_current(ygg::uint_t(index));
        return m_graph.cardinality(index);
    }
    /// Changes of nodes updated from the transition into the current state.
    template<ConceptOrRoleTag Category>
    const DenotationDelta<Category>& get_delta(EvaluationIndex<Category> index) const&
    {
        require_exact(ygg::uint_t(index));
        return m_graph.change(index);
    }
    template<ConceptOrRoleTag Category>
    const DenotationDelta<Category>& get_delta(EvaluationIndex<Category>) const&& = delete;
    const auto& get_delta(QueryEvaluationIndex index) const&
    {
        require_exact(ygg::uint_t(index));
        return m_graph.change(index);
    }
    const auto& get_delta(QueryEvaluationIndex) const&& = delete;
    template<BooleanOrNumericalTag Category>
    bool changed(EvaluationIndex<Category> index) const
    {
        require_exact(ygg::uint_t(index));
        return m_graph.node(index).changed();
    }
};

template<FamilyTag Family, tyr::TaskKind Kind, typename Context>
class StateEvaluation
{
    IncrementalEvaluation<Family, Kind>* m_evaluation;
    uint64_t m_state;
    Context m_context;
    const Delta<Family>* m_delta;  ///< The transition into the state, if known.
    ygg::database::Workspace<QueryValues>* m_workspace;

    friend class IncrementalEvaluation<Family, Kind>;
    StateEvaluation(IncrementalEvaluation<Family, Kind>& evaluation,
                    Context context,
                    const Delta<Family>* delta,
                    ygg::database::Workspace<QueryValues>* workspace) :
        m_evaluation(&evaluation),
        m_state(evaluation.m_state),
        m_context(std::move(context)),
        m_delta(delta),
        m_workspace(workspace)
    {
    }
    IncrementalEvaluation<Family, Kind>& current(ygg::uint_t index)
    {
        if (m_state != m_evaluation->m_state)
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
    const auto& evaluate(QueryEvaluationIndex index) { return current(ygg::uint_t(index)).get_result(index); }
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
    const auto& delta(QueryEvaluationIndex index) { return current(ygg::uint_t(index)).get_delta(index); }
    template<BooleanOrNumericalTag Category>
    bool changed(EvaluationIndex<Category> index)
    {
        return current(ygg::uint_t(index)).changed(index);
    }
};

template<FamilyTag Family, tyr::TaskKind Kind>
template<typename Context>
void IncrementalEvaluation<Family, Kind>::demand(ygg::uint_t index,
                                                  Context& context,
                                                  const Delta<Family>* delta,
                                                  ygg::database::Workspace<QueryValues>* workspace)
{
    if (m_nodes.at(index).state == m_state)
        return;
    std::visit(
        [&](auto& evaluator)
        {
            // Finish children before borrowing the shared operation workspace.
            auto children_exact = true;
            for (const auto child : evaluator.get_dependencies())
            {
                demand(child, context, delta, workspace);
                children_exact = children_exact && m_nodes[child].exact;
            }
            auto& node = m_nodes[index];
            const auto update = delta && node.state != 0 && node.state + 1 == m_state && children_exact;
            node = NodeState {};
            if (update)
                evaluator.update(m_graph, *delta, *workspace);
            else if constexpr (std::same_as<Context, NoContext>)
                throw std::logic_error("Incremental evaluation: initialize with the state's inputs before updating.");
            else
                evaluator.initialize(m_graph, context);
            node = NodeState { m_state, update };
        },
        m_graph.m_nodes[index]);
}

/// Convenience owner for one root, evaluated eagerly in each state. Use IncrementalEvaluation to share children
/// across roots.
template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category>
class Evaluator
{
    IncrementalEvaluation<Family, Kind> m_evaluation;
    EvaluationIndex<Category> m_root;

public:
    Evaluator(const tyr::planning::Task<Kind>& task, FamilyConstructorView<Family, Category> expression) :
        m_evaluation(task, { expression }),
        m_root(m_evaluation.graph().get_index(expression))
    {
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(Context& context)
    {
        m_evaluation.enter(context).evaluate(m_root);
    }
    /// Throws std::logic_error unless the root can be updated from the delta; then initialize() is required.
    void update(const Delta<Family>& delta, ygg::database::Workspace<QueryValues>& workspace) { m_evaluation.enter(delta, workspace).evaluate(m_root); }
    auto get_result() const& { return m_evaluation.get_result(m_root); }
    auto get_result() const&& = delete;
    const auto& get_delta() const&
        requires ConceptOrRoleTag<Category>
    {
        return m_evaluation.get_delta(m_root);
    }
    const auto& get_delta() const&& = delete;
    size_t size() const
        requires ConceptOrRoleTag<Category>
    {
        return m_evaluation.size(m_root);
    }
    bool changed() const
        requires BooleanOrNumericalTag<Category>
    {
        return m_evaluation.changed(m_root);
    }
};

template<FamilyTag Family, tyr::TaskKind Kind>
class QueryEvaluator
{
    IncrementalEvaluation<Family, Kind> m_evaluation;
    QueryEvaluationIndex m_root;

public:
    QueryEvaluator(const tyr::planning::Task<Kind>& task, FamilyQueryView<Family> expression) :
        m_evaluation(task, { expression }),
        m_root(m_evaluation.graph().get_index(expression))
    {
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(Context& context)
    {
        m_evaluation.enter(context).evaluate(m_root);
    }
    void update(const Delta<Family>& delta, ygg::database::Workspace<QueryValues>& workspace) { m_evaluation.enter(delta, workspace).evaluate(m_root); }
    const auto& get_result() const& { return m_evaluation.get_result(m_root); }
    const auto& get_result() const&& = delete;
    const auto& get_delta() const& { return m_evaluation.get_delta(m_root); }
    const auto& get_delta() const&& = delete;
};

}  // namespace runir::kr::dl::semantics::incremental

#endif
