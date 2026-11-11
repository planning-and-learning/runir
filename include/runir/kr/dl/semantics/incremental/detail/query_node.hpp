#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_QUERY_NODE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_QUERY_NODE_HPP_

#include "runir/kr/dl/semantics/incremental/detail/evaluators/must_pass.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/query.hpp"

#include <algorithm>
#include <optional>
#include <span>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <variant>
#include <vector>

namespace runir::kr::dl::semantics::incremental::detail
{

/// One prepared query constructor. Operands identify preceding graph nodes;
/// each operation owns its result and reports only the last update's changes.
template<tyr::TaskKind Kind>
class QueryNode
{
public:
    QueryNode(QueryView<> expression, GraphBuilder<Kind>& graph);
    /// A concept or role lifted into a relation with the given columns; it has no query expression.
    template<ConceptOrRoleTag Category>
    QueryNode(const ygg::FreeRelationRepository& layer, EvaluationIndex<Category> argument, std::span<const ygg::ColumnLayout> columns) :
        m_dependencies { ygg::uint_t(argument.value) },
        m_operation(QueryFromDenotationEvaluator<Kind, Category>(layer, argument, columns))
    {
    }
    [[nodiscard]] std::optional<QueryView<>> get_expression() const noexcept { return m_expression; }
    /// Whether this node lifts argument into a relation with exactly these columns.
    template<ConceptOrRoleTag Category>
    bool lifts(EvaluationIndex<Category> argument, std::span<const ygg::ColumnLayout> columns) const
    {
        const auto* lifted = std::get_if<QueryFromDenotationEvaluator<Kind, Category>>(&m_operation);
        return !m_expression && lifted && lifted->argument == argument && std::ranges::equal(lifted->result.columns(), columns);
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context& context);
    void update(EvaluationGraph<Kind>& graph, const Delta& delta, tyr::datalog::database::Workspace& workspace);
    [[nodiscard]] std::span<const ygg::uint_t> get_dependencies() const noexcept { return m_dependencies; }
    [[nodiscard]] ygg::FreeBuilderRelationView get_result() const;
    [[nodiscard]] const tyr::datalog::database::incremental::Delta& get_delta() const;

private:
    using Operation = std::variant<QueryStaticEvaluator<Kind>,
                                   QueryAtomicEvaluator<Kind, tyr::formalism::FluentTag>,
                                   QueryAtomicEvaluator<Kind, tyr::formalism::DerivedTag>,
                                   QueryProjectionEvaluator<Kind>,
                                   QueryJoinEvaluator<Kind>,
                                   QueryGenericJoinEvaluator<Kind>,
                                   QueryFromDenotationEvaluator<Kind, ConceptTag>,
                                   QueryFromDenotationEvaluator<Kind, RoleTag>,
                                   QuerySetCombinationEvaluator<Kind, QueryUnionTag>,
                                   QuerySetCombinationEvaluator<Kind, QueryDifferenceTag>,
                                   QuerySelectionEvaluator<Kind, QueryRenameTag>,
                                   QuerySelectionEvaluator<Kind, QuerySelectEqualTag>,
                                   QuerySelectionEvaluator<Kind, QuerySelectValueTag>,
                                   QueryMustPassEvaluator<Kind>>;

    std::optional<QueryView<>> m_expression;
    urd::Store<urd::sequence<ygg::uint_t>> m_dependencies;
    Operation m_operation;
    Operation prepare_operation(QueryView<> expression, GraphBuilder<Kind>& graph);
};

template<tyr::TaskKind Kind>
QueryNode<Kind>::QueryNode(QueryView<> expression, GraphBuilder<Kind>& graph) : m_expression(expression), m_operation(prepare_operation(expression, graph))
{
}

template<tyr::TaskKind Kind>
template<StateEvaluationContextConcept<Kind> Context>
void QueryNode<Kind>::initialize(EvaluationGraph<Kind>& graph, Context& context)
{
    std::visit([&](auto& operation) { operation.initialize(graph, context); }, m_operation);
}

template<tyr::TaskKind Kind>
void QueryNode<Kind>::update(EvaluationGraph<Kind>& graph, const Delta& delta, tyr::datalog::database::Workspace& workspace)
{
    std::visit([&](auto& operation) { operation.update(graph, delta, workspace); }, m_operation);
}

template<tyr::TaskKind Kind>
ygg::FreeBuilderRelationView QueryNode<Kind>::get_result() const
{
    return std::visit([](const auto& operation) -> ygg::FreeBuilderRelationView { return operation.get_result(); }, m_operation);
}

template<tyr::TaskKind Kind>
const tyr::datalog::database::incremental::Delta& QueryNode<Kind>::get_delta() const
{
    return std::visit([](const auto& operation) -> const auto& { return operation.get_delta(); }, m_operation);
}

template<tyr::TaskKind Kind>
auto QueryNode<Kind>::prepare_operation(QueryView<> expression, GraphBuilder<Kind>& graph) -> Operation
{
    const auto& layer = graph.layer();
    if (expression.is_static())
        return QueryStaticEvaluator<Kind>(layer, expression);
    return std::visit(
        [&]<typename Tag>(QueryView<Tag> concrete) -> Operation
        {
            if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::FluentTag>>)
                return QueryAtomicEvaluator<Kind, tyr::formalism::FluentTag>(layer, concrete);
            else if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::DerivedTag>>)
                return QueryAtomicEvaluator<Kind, tyr::formalism::DerivedTag>(layer, concrete);
            else if constexpr (std::same_as<Tag, QueryProjectTag>)
                return QueryProjectionEvaluator<Kind>(layer, graph.prepare(concrete.get_arg(), m_dependencies), concrete.data().plan);
            else if constexpr (std::same_as<Tag, QueryJoinTag>)
                return QueryJoinEvaluator<Kind>(layer,
                                                graph.prepare(concrete.get_lhs(), m_dependencies),
                                                graph.prepare(concrete.get_rhs(), m_dependencies),
                                                concrete.data().plan);
            else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
            {
                auto args = urd::Store<urd::sequence<QueryEvaluationIndex>> {};
                for (const auto arg : concrete.get_args())
                    args.push_back(graph.prepare(arg, m_dependencies));
                return QueryGenericJoinEvaluator<Kind>(layer, std::move(args), concrete.data().plan);
            }
            else if constexpr (std::same_as<Tag, QueryConceptTag>)
                return QueryFromDenotationEvaluator<Kind, ConceptTag>(layer, graph.prepare(concrete.get_arg(), m_dependencies), concrete.get_schema());
            else if constexpr (std::same_as<Tag, QueryRoleTag>)
                return QueryFromDenotationEvaluator<Kind, RoleTag>(layer, graph.prepare(concrete.get_arg(), m_dependencies), concrete.get_schema());
            else if constexpr (std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
                return QuerySetCombinationEvaluator<Kind, Tag>(layer,
                                                               graph.prepare(concrete.get_lhs(), m_dependencies),
                                                               graph.prepare(concrete.get_rhs(), m_dependencies),
                                                               concrete.get_schema());
            else if constexpr (std::same_as<Tag, QueryRenameTag> || std::same_as<Tag, QuerySelectEqualTag> || std::same_as<Tag, QuerySelectValueTag>)
                return QuerySelectionEvaluator<Kind, Tag>(layer, graph.prepare(concrete.get_arg(), m_dependencies), concrete);
            else if constexpr (std::same_as<Tag, QueryMustPassTag>)
                return QueryMustPassEvaluator<Kind>(concrete, graph, m_dependencies);
            else
                throw std::logic_error("Incremental query: a static constructor was marked dynamic.");
        },
        expression.get_variant());
}

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
