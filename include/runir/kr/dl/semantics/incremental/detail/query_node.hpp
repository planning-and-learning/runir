#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_QUERY_NODE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_QUERY_NODE_HPP_

#include "runir/kr/dl/semantics/incremental/detail/evaluators/must_pass.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/query.hpp"

#include <algorithm>
#include <optional>
#include <span>
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
    QueryNode(EvaluationIndex<Category> argument, std::span<const ygg::database::ColumnLayout> columns) :
        m_dependencies { ygg::uint_t(argument) },
        m_operation(QueryFromDenotationEvaluator<Kind, Category>(argument, columns))
    {
    }
    std::optional<QueryView<>> get_expression() const noexcept { return m_expression; }
    /// Whether this node lifts argument into a relation with exactly these columns.
    template<ConceptOrRoleTag Category>
    bool lifts(EvaluationIndex<Category> argument, std::span<const ygg::database::ColumnLayout> columns) const
    {
        const auto* lifted = std::get_if<QueryFromDenotationEvaluator<Kind, Category>>(&m_operation);
        return !m_expression && lifted && lifted->argument == argument && std::ranges::equal(lifted->result.columns().span(), columns);
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context& context);
    void update(EvaluationGraph<Kind>& graph, const Delta& delta, ygg::database::Workspace& workspace);
    std::span<const ygg::uint_t> get_dependencies() const noexcept { return m_dependencies; }
    const ygg::Builder<ygg::database::Relation>& get_result() const;
    const ygg::database::incremental::Delta& get_delta() const;

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
    std::vector<ygg::uint_t> m_dependencies;
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
void QueryNode<Kind>::update(EvaluationGraph<Kind>& graph, const Delta& delta, ygg::database::Workspace& workspace)
{
    std::visit([&](auto& operation) { operation.update(graph, delta, workspace); }, m_operation);
}

template<tyr::TaskKind Kind>
const ygg::Builder<ygg::database::Relation>& QueryNode<Kind>::get_result() const
{
    return std::visit([](const auto& operation) -> const auto& { return operation.get_result(); }, m_operation);
}

template<tyr::TaskKind Kind>
const ygg::database::incremental::Delta& QueryNode<Kind>::get_delta() const
{
    return std::visit([](const auto& operation) -> const auto& { return operation.get_delta(); }, m_operation);
}

template<tyr::TaskKind Kind>
auto QueryNode<Kind>::prepare_operation(QueryView<> expression, GraphBuilder<Kind>& graph) -> Operation
{
    if (expression.is_static())
        return QueryStaticEvaluator<Kind>(expression);
    return ygg::visit(
        [&]<typename Tag>(QueryView<Tag> concrete) -> Operation
        {
            if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::FluentTag>>)
                return QueryAtomicEvaluator<Kind, tyr::formalism::FluentTag>(concrete);
            else if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::DerivedTag>>)
                return QueryAtomicEvaluator<Kind, tyr::formalism::DerivedTag>(concrete);
            else if constexpr (std::same_as<Tag, QueryProjectTag>)
                return QueryProjectionEvaluator<Kind>(graph.prepare(concrete.get_arg(), m_dependencies), concrete.get_data().plan);
            else if constexpr (std::same_as<Tag, QueryJoinTag>)
                return QueryJoinEvaluator<Kind>(graph.prepare(concrete.get_lhs(), m_dependencies),
                                                graph.prepare(concrete.get_rhs(), m_dependencies),
                                                concrete.get_data().plan);
            else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
            {
                auto args = std::vector<QueryEvaluationIndex> {};
                for (const auto arg : concrete.get_args())
                    args.push_back(graph.prepare(arg, m_dependencies));
                return QueryGenericJoinEvaluator<Kind>(std::move(args), concrete.get_data().plan);
            }
            else if constexpr (std::same_as<Tag, QueryConceptTag>)
                return QueryFromDenotationEvaluator<Kind, ConceptTag>(graph.prepare(concrete.get_arg(), m_dependencies), concrete.get_schema().span());
            else if constexpr (std::same_as<Tag, QueryRoleTag>)
                return QueryFromDenotationEvaluator<Kind, RoleTag>(graph.prepare(concrete.get_arg(), m_dependencies), concrete.get_schema().span());
            else if constexpr (std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
                return QuerySetCombinationEvaluator<Kind, Tag>(graph.prepare(concrete.get_lhs(), m_dependencies),
                                                               graph.prepare(concrete.get_rhs(), m_dependencies),
                                                               concrete.get_schema().span());
            else if constexpr (std::same_as<Tag, QueryRenameTag> || std::same_as<Tag, QuerySelectEqualTag> || std::same_as<Tag, QuerySelectValueTag>)
                return QuerySelectionEvaluator<Kind, Tag>(graph.prepare(concrete.get_arg(), m_dependencies), concrete);
            else if constexpr (std::same_as<Tag, QueryMustPassTag>)
                return QueryMustPassEvaluator<Kind>(concrete, graph, m_dependencies);
            else
                throw std::logic_error("Incremental query: a static constructor was marked dynamic.");
        },
        expression.get_variant());
}

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
