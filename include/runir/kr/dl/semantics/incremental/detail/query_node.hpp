#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_QUERY_NODE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_QUERY_NODE_HPP_

#include "runir/kr/dl/semantics/incremental/detail/evaluators/query.hpp"

#include <span>
#include <variant>
#include <vector>

namespace runir::kr::dl::semantics::incremental::detail
{

/// One prepared query constructor. Operands identify preceding graph nodes;
/// each operation owns its result and reports only the last update's changes.
template<FamilyTag Family, tyr::TaskKind Kind>
class QueryNode
{
public:
    QueryNode(FamilyQueryView<Family> expression, EvaluationGraph<Family, Kind>& graph);
    FamilyQueryView<Family> get_expression() const noexcept { return m_expression; }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context& context);
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>& delta, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace);
    std::span<const ygg::uint_t> get_dependencies() const noexcept { return m_dependencies; }
    const ygg::Builder<ygg::database::Relation<ygg::Index<tyr::formalism::Object>>>& get_result() const;
    const ygg::database::incremental::Delta<ygg::Index<tyr::formalism::Object>>& get_delta() const;

private:
    using Operation = std::variant<QueryStaticEvaluator<Family, Kind>,
                                   QueryAtomicEvaluator<Family, Kind, tyr::formalism::FluentTag>,
                                   QueryAtomicEvaluator<Family, Kind, tyr::formalism::DerivedTag>,
                                   QueryProjectionEvaluator<Family, Kind>,
                                   QueryJoinEvaluator<Family, Kind>,
                                   QueryFromDenotationEvaluator<Family, Kind, ConceptTag>,
                                   QueryFromDenotationEvaluator<Family, Kind, RoleTag>,
                                   QuerySetCombinationEvaluator<Family, Kind, QueryUnionTag>,
                                   QuerySetCombinationEvaluator<Family, Kind, QueryDifferenceTag>,
                                   QuerySelectionEvaluator<Family, Kind, QueryRenameTag>,
                                   QuerySelectionEvaluator<Family, Kind, QuerySelectEqualTag>,
                                   QuerySelectionEvaluator<Family, Kind, QuerySelectValueTag>>;

    FamilyQueryView<Family> m_expression;
    std::vector<ygg::uint_t> m_dependencies;
    Operation m_operation;
    Operation prepare_operation(FamilyQueryView<Family> expression, EvaluationGraph<Family, Kind>& graph);
};

template<FamilyTag Family, tyr::TaskKind Kind>
QueryNode<Family, Kind>::QueryNode(FamilyQueryView<Family> expression, EvaluationGraph<Family, Kind>& graph) :
    m_expression(expression),
    m_operation(prepare_operation(expression, graph))
{
}

template<FamilyTag Family, tyr::TaskKind Kind>
template<StateEvaluationContextConcept<Family, Kind> Context>
void QueryNode<Family, Kind>::initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
{
    std::visit([&](auto& operation) { operation.initialize(graph, context); }, m_operation);
}

template<FamilyTag Family, tyr::TaskKind Kind>
void QueryNode<Family, Kind>::update(EvaluationGraph<Family, Kind>& graph,
                                     const Delta<Family>& delta,
                                     ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace)
{
    std::visit([&](auto& operation) { operation.update(graph, delta, workspace); }, m_operation);
}

template<FamilyTag Family, tyr::TaskKind Kind>
const ygg::Builder<ygg::database::Relation<ygg::Index<tyr::formalism::Object>>>& QueryNode<Family, Kind>::get_result() const
{
    return std::visit([](const auto& operation) -> const auto& { return operation.get_result(); }, m_operation);
}

template<FamilyTag Family, tyr::TaskKind Kind>
const ygg::database::incremental::Delta<ygg::Index<tyr::formalism::Object>>& QueryNode<Family, Kind>::get_delta() const
{
    return std::visit([](const auto& operation) -> const auto& { return operation.get_delta(); }, m_operation);
}

template<FamilyTag Family, tyr::TaskKind Kind>
auto QueryNode<Family, Kind>::prepare_operation(FamilyQueryView<Family> expression, EvaluationGraph<Family, Kind>& graph) -> Operation
{
    if (expression.is_static())
        return QueryStaticEvaluator<Family, Kind>(expression);
    return ygg::visit(
        [&]<typename Tag>(FamilyQueryView<Family, Tag> concrete) -> Operation
        {
            if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::FluentTag>>)
                return QueryAtomicEvaluator<Family, Kind, tyr::formalism::FluentTag>(concrete);
            else if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::DerivedTag>>)
                return QueryAtomicEvaluator<Family, Kind, tyr::formalism::DerivedTag>(concrete);
            else if constexpr (std::same_as<Tag, QueryProjectTag>)
                return QueryProjectionEvaluator<Family, Kind>(graph.prepare(concrete.get_arg(), m_dependencies), concrete.get_data().plan);
            else if constexpr (std::same_as<Tag, QueryJoinTag>)
                return QueryJoinEvaluator<Family, Kind>(graph.prepare(concrete.get_lhs(), m_dependencies),
                                                        graph.prepare(concrete.get_rhs(), m_dependencies),
                                                        concrete.get_data().plan);
            else if constexpr (std::same_as<Tag, QueryConceptTag>)
                return QueryFromDenotationEvaluator<Family, Kind, ConceptTag>(graph.prepare(concrete.get_arg(), m_dependencies), concrete.get_schema().span());
            else if constexpr (std::same_as<Tag, QueryRoleTag>)
                return QueryFromDenotationEvaluator<Family, Kind, RoleTag>(graph.prepare(concrete.get_arg(), m_dependencies), concrete.get_schema().span());
            else if constexpr (std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
                return QuerySetCombinationEvaluator<Family, Kind, Tag>(graph.prepare(concrete.get_lhs(), m_dependencies),
                                                                       graph.prepare(concrete.get_rhs(), m_dependencies),
                                                                       concrete.get_schema().span());
            else if constexpr (std::same_as<Tag, QueryRenameTag> || std::same_as<Tag, QuerySelectEqualTag> || std::same_as<Tag, QuerySelectValueTag>)
                return QuerySelectionEvaluator<Family, Kind, Tag>(graph.prepare(concrete.get_arg(), m_dependencies), concrete);
            else
                throw std::logic_error("Incremental query: a static constructor was marked dynamic.");
        },
        expression.get_variant());
}

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
