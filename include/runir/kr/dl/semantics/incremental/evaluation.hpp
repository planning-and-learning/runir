#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_EVALUATION_HPP_

#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/detail/feature_node.hpp"
#include "runir/kr/dl/semantics/incremental/detail/query_node.hpp"

#include <algorithm>
#include <initializer_list>
#include <optional>
#include <span>
#include <stdexcept>
#include <variant>
#include <vector>

namespace runir::kr::dl::semantics::incremental
{

/// Prepared dependency graph shared by any number of feature roots.
/// Construction prepares all roots; children precede their consumers.
/// The task and expression repositories must outlive this graph.
/// Reinitialize on invocation changes, retaining task-static results and buffers.
/// Indices belong to this graph. Result views borrow it and its task repository;
/// mutation invalidates iterators, and moving the graph invalidates borrowed views.
/// Ext input deltas must belong to the same module invocation.
template<FamilyTag Family, tyr::TaskKind Kind>
class EvaluationGraph
{
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
    template<FamilyTag, tyr::TaskKind, CategoryTag>
    friend struct detail::ProjectionEvaluator;
    template<FamilyTag, tyr::TaskKind>
    friend struct detail::QueryProjectionEvaluator;
    template<FamilyTag, tyr::TaskKind>
    friend struct detail::QueryJoinEvaluator;
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
    enum class Mode
    {
        UNINITIALIZED,
        EAGER,
        DEMAND
    };
    Mode m_mode = Mode::UNINITIALIZED;
    std::vector<bool> m_active;

    template<CategoryTag Category>
    const auto& node(EvaluationIndex<Category> index) const
    {
        return std::get<detail::FeatureNode<Family, Kind, Category>>(m_nodes.at(ygg::uint_t(index)));
    }
    const auto& node(QueryEvaluationIndex index) const { return std::get<detail::QueryNode<Family, Kind>>(m_nodes.at(ygg::uint_t(index))); }

    template<CategoryTag Category>
    std::optional<EvaluationIndex<Category>> find_node(FamilyConstructorView<Family, Category> expression) const;
    std::optional<QueryEvaluationIndex> find_node(FamilyQueryView<Family> expression) const;

    // Prepared evaluators read already updated children while the public baseline is invalid.
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

    void require_initialized() const;
    void require_eager() const;
    void require_ready(ygg::uint_t index) const;
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void evaluate_node(ygg::uint_t index, Context& context);
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void demand(ygg::uint_t index, Context& context);
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

    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(Context& context);
    /// Failed updates invalidate the graph until initialize() replaces its baseline.
    void update(const Delta<Family>& delta, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace);

    /// Start a demand-driven invocation, retaining buffers and task-static values.
    void reset() noexcept;
    /// Maintain previously demanded nodes in the next state; inactive nodes stay untouched.
    /// A failed advance invalidates the graph until reset() starts a fresh invocation.
    void advance(const Delta<Family>& delta, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace);
    /// Initialize a root and its inactive dependencies on first demand. Later states
    /// maintain them through advance(); exact deltas are exposed only in eager mode.
    template<CategoryTag Category, StateEvaluationContextConcept<Family, Kind> Context>
    BorrowedDenotationView<Category> evaluate(EvaluationIndex<Category> index, Context& context)
    {
        demand(ygg::uint_t(index), context);
        return get_result(index);
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    const auto& evaluate(QueryEvaluationIndex index, Context& context)
    {
        demand(ygg::uint_t(index), context);
        return get_result(index);
    }

    template<CategoryTag Category>
    BorrowedDenotationView<Category> get_result(EvaluationIndex<Category> index) const&
    {
        require_ready(ygg::uint_t(index));
        return result(index);
    }
    template<CategoryTag Category>
    BorrowedDenotationView<Category> get_result(EvaluationIndex<Category>) const&& = delete;
    const auto& get_result(QueryEvaluationIndex index) const&
    {
        require_ready(ygg::uint_t(index));
        return result(index);
    }
    const auto& get_result(QueryEvaluationIndex) const&& = delete;

    template<ConceptOrRoleTag Category>
    const DenotationDelta<Category>& get_delta(EvaluationIndex<Category> index) const&
    {
        require_eager();
        return change(index);
    }
    template<ConceptOrRoleTag Category>
    const DenotationDelta<Category>& get_delta(EvaluationIndex<Category>) const&& = delete;
    const auto& get_delta(QueryEvaluationIndex index) const&
    {
        require_eager();
        return change(index);
    }
    const auto& get_delta(QueryEvaluationIndex) const&& = delete;

    template<ConceptOrRoleTag Category>
    size_t size(EvaluationIndex<Category> index) const
    {
        require_ready(ygg::uint_t(index));
        return cardinality(index);
    }
    size_t size(QueryEvaluationIndex index) const
    {
        require_ready(ygg::uint_t(index));
        return cardinality(index);
    }
    template<BooleanOrNumericalTag Category>
    bool changed(EvaluationIndex<Category> index) const
    {
        require_eager();
        return node(index).changed();
    }
    size_t node_count() const noexcept { return m_nodes.size(); }
    /// False before initialization or after an evaluation failure.
    bool is_valid() const noexcept { return m_mode != Mode::UNINITIALIZED; }
};

/// Convenience owner for one root. Use EvaluationGraph to share children across roots.
template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category>
class Evaluator
{
    EvaluationGraph<Family, Kind> m_graph;
    EvaluationIndex<Category> m_root;

public:
    Evaluator(const tyr::planning::Task<Kind>& task, FamilyConstructorView<Family, Category> expression) :
        m_graph(task, { expression }),
        m_root(m_graph.get_index(expression))
    {
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(Context& context)
    {
        m_graph.initialize(context);
    }
    void update(const Delta<Family>& delta, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace) { m_graph.update(delta, workspace); }
    auto get_result() const& { return m_graph.get_result(m_root); }
    auto get_result() const&& = delete;
    const auto& get_delta() const&
        requires ConceptOrRoleTag<Category>
    {
        return m_graph.get_delta(m_root);
    }
    const auto& get_delta() const&& = delete;
    size_t size() const
        requires ConceptOrRoleTag<Category>
    {
        return m_graph.size(m_root);
    }
    bool changed() const
        requires BooleanOrNumericalTag<Category>
    {
        return m_graph.changed(m_root);
    }
};

template<FamilyTag Family, tyr::TaskKind Kind>
class QueryEvaluator
{
    EvaluationGraph<Family, Kind> m_graph;
    QueryEvaluationIndex m_root;

public:
    QueryEvaluator(const tyr::planning::Task<Kind>& task, FamilyQueryView<Family> expression) :
        m_graph(task, { expression }),
        m_root(m_graph.get_index(expression))
    {
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(Context& context)
    {
        m_graph.initialize(context);
    }
    void update(const Delta<Family>& delta, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace) { m_graph.update(delta, workspace); }
    const auto& get_result() const& { return m_graph.get_result(m_root); }
    const auto& get_result() const&& = delete;
    const auto& get_delta() const& { return m_graph.get_delta(m_root); }
    const auto& get_delta() const&& = delete;
};

template<FamilyTag Family, tyr::TaskKind Kind>
void EvaluationGraph<Family, Kind>::require_initialized() const
{
    if (m_mode == Mode::UNINITIALIZED)
        throw std::logic_error("Incremental evaluation: initialize before reading or updating results.");
}

template<FamilyTag Family, tyr::TaskKind Kind>
void EvaluationGraph<Family, Kind>::require_eager() const
{
    if (m_mode != Mode::EAGER)
        throw std::logic_error("Incremental evaluation: exact deltas require initialize/update evaluation.");
}

template<FamilyTag Family, tyr::TaskKind Kind>
void EvaluationGraph<Family, Kind>::require_ready(ygg::uint_t index) const
{
    require_initialized();
    if (m_mode == Mode::DEMAND && !m_active.at(index))
        throw std::logic_error("Incremental evaluation: demand this result before reading it.");
}

template<FamilyTag Family, tyr::TaskKind Kind>
void EvaluationGraph<Family, Kind>::reset() noexcept
{
    std::fill(m_active.begin(), m_active.end(), false);
    m_mode = Mode::DEMAND;
}

template<FamilyTag Family, tyr::TaskKind Kind>
void EvaluationGraph<Family, Kind>::advance(const Delta<Family>& delta, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace)
{
    if (m_mode != Mode::DEMAND)
        throw std::logic_error("Incremental evaluation: reset before advancing demand evaluation.");
    m_mode = Mode::UNINITIALIZED;
    // Activated nodes include all their dependencies, which precede them in m_nodes.
    for (size_t i = 0; i < m_nodes.size(); ++i)
        if (m_active[i])
            std::visit([&](auto& evaluator) { evaluator.update(*this, delta, workspace); }, m_nodes[i]);
    m_mode = Mode::DEMAND;
}

template<FamilyTag Family, tyr::TaskKind Kind>
template<StateEvaluationContextConcept<Family, Kind> Context>
void EvaluationGraph<Family, Kind>::demand(ygg::uint_t index, Context& context)
{
    if (m_mode != Mode::DEMAND)
        throw std::logic_error("Incremental evaluation: reset before demand evaluation.");
    if (&context.get_state().get_task() != &m_task)
        throw std::invalid_argument("Incremental evaluation: graph belongs to a different task.");
    try
    {
        evaluate_node(index, context);
    }
    catch (...)
    {
        m_mode = Mode::UNINITIALIZED;
        throw;
    }
}

template<FamilyTag Family, tyr::TaskKind Kind>
template<StateEvaluationContextConcept<Family, Kind> Context>
void EvaluationGraph<Family, Kind>::evaluate_node(ygg::uint_t index, Context& context)
{
    if (m_active.at(index))
        return;
    std::visit(
        [&](auto& evaluator)
        {
            // Finish children before borrowing the shared operation workspace.
            for (const auto child : evaluator.get_dependencies())
                evaluate_node(child, context);
            evaluator.initialize(*this, context);
        },
        m_nodes.at(index));
    m_active[index] = true;
}

template<FamilyTag Family, tyr::TaskKind Kind>
EvaluationGraph<Family, Kind>::EvaluationGraph(const tyr::planning::Task<Kind>& task, std::span<const EvaluationRoot<Family>> roots) : m_task(task)
{
    m_set_workspace.initialize(ygg::to_uint_t(task.get_task().get_num_objects()));
    for (const auto& root : roots)
        std::visit([&](auto expression) { prepare(expression); }, root);
    m_active.resize(m_nodes.size());
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

template<FamilyTag Family, tyr::TaskKind Kind>
template<StateEvaluationContextConcept<Family, Kind> Context>
void EvaluationGraph<Family, Kind>::initialize(Context& context)
{
    const auto* task = &context.get_state().get_task();
    if (&m_task != task)
        throw std::invalid_argument("Incremental evaluation: graph belongs to a different task.");
    m_mode = Mode::UNINITIALIZED;
    for (auto& node : m_nodes)
        std::visit([&](auto& evaluator) { evaluator.initialize(*this, context); }, node);
    m_mode = Mode::EAGER;
}

template<FamilyTag Family, tyr::TaskKind Kind>
void EvaluationGraph<Family, Kind>::update(const Delta<Family>& delta, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace)
{
    require_eager();
    m_mode = Mode::UNINITIALIZED;
    for (auto& node : m_nodes)
        std::visit([&](auto& evaluator) { evaluator.update(*this, delta, workspace); }, node);
    m_mode = Mode::EAGER;
}

}  // namespace runir::kr::dl::semantics::incremental

#endif
