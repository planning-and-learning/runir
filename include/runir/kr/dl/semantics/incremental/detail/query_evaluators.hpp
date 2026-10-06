#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_QUERY_EVALUATORS_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_QUERY_EVALUATORS_HPP_

#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/atomic_query.hpp"

#include <array>
#include <span>
#include <variant>

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
    const ygg::Builder<ygg::database::Relation<ygg::Index<tyr::formalism::Object>>>& get_result() const;
    const ygg::database::incremental::Delta<ygg::Index<tyr::formalism::Object>>& get_delta() const;

private:
    class QueryValue
    {
        ygg::Builder<ygg::database::Relation<ygg::Index<tyr::formalism::Object>>> m_result;
        ygg::database::incremental::Delta<ygg::Index<tyr::formalism::Object>> m_delta;

    protected:
        explicit QueryValue(std::span<const ygg::Index<ygg::database::Column>> columns) : m_result(columns), m_delta(columns) {}
        void clear() noexcept
        {
            m_result.clear();
            m_delta.clear();
        }
        void clear_delta() noexcept { m_delta.clear(); }
        template<ygg::database::RelationViewConcept<ygg::Index<tyr::formalism::Object>> Source>
        void assign(const Source& source)
        {
            ygg::database::assign(m_result, source);
            m_delta.clear();
        }
        void set(std::span<const ygg::Index<tyr::formalism::Object>> row, bool present);

    public:
        const auto& get_result() const noexcept { return m_result; }
        const auto& get_delta() const noexcept { return m_delta; }
    };

    struct Static : QueryValue
    {
        FamilyQueryView<Family> expression;
        bool initialized = false;

        explicit Static(FamilyQueryView<Family> expression) : QueryValue(expression.get_schema().span()), expression(expression) {}
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>&, Context& context)
        {
            if (!initialized)
            {
                this->assign(semantics::evaluate<Kind>(expression, context));
                initialized = true;
            }
            this->clear_delta();
        }
        void update(EvaluationGraph<Family, Kind>&, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&) {}
    };

    template<tyr::formalism::FactKind Fact>
    struct Atomic : AtomicQueryEvaluator<Fact>
    {
        explicit Atomic(FamilyQueryView<Family, AtomicStateTag<Fact>> expression) : AtomicQueryEvaluator<Fact>(expression) {}
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>&, Context& context)
        {
            AtomicQueryEvaluator<Fact>::template initialize<Kind>(context.get_state());
        }
        void update(EvaluationGraph<Family, Kind>&, const Delta<Family>& delta, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            if constexpr (std::same_as<Fact, tyr::formalism::FluentTag>)
                AtomicQueryEvaluator<Fact>::update(delta.added.fluent_atoms, delta.removed.fluent_atoms);
            else
                AtomicQueryEvaluator<Fact>::update(delta.added.derived_atoms, delta.removed.derived_atoms);
        }
    };

    struct Projection : ygg::database::incremental::ProjectionEvaluator<ygg::Index<tyr::formalism::Object>>
    {
        QueryEvaluationIndex argument;

        Projection(QueryEvaluationIndex argument, const ygg::database::ProjectionPlan& plan) :
            ygg::database::incremental::ProjectionEvaluator<ygg::Index<tyr::formalism::Object>>(plan),
            argument(argument)
        {
        }
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
        {
            ygg::database::incremental::ProjectionEvaluator<ygg::Index<tyr::formalism::Object>>::initialize(graph.result(argument),
                                                                                                            context.get_workspace().get_database_workspace());
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace)
        {
            const auto& delta = graph.change(argument);
            ygg::database::incremental::ProjectionEvaluator<ygg::Index<tyr::formalism::Object>>::update(delta.added, delta.removed, workspace);
        }
    };

    struct Join : ygg::database::incremental::JoinEvaluator<ygg::Index<tyr::formalism::Object>>
    {
        QueryEvaluationIndex lhs;
        QueryEvaluationIndex rhs;

        Join(QueryEvaluationIndex lhs, QueryEvaluationIndex rhs, const ygg::database::JoinPlan& plan) :
            ygg::database::incremental::JoinEvaluator<ygg::Index<tyr::formalism::Object>>(plan),
            lhs(lhs),
            rhs(rhs)
        {
        }
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
        {
            ygg::database::incremental::JoinEvaluator<ygg::Index<tyr::formalism::Object>>::initialize(graph.result(lhs),
                                                                                                      graph.result(rhs),
                                                                                                      context.get_workspace().get_database_workspace());
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace)
        {
            const auto& left = graph.change(lhs);
            const auto& right = graph.change(rhs);
            ygg::database::incremental::JoinEvaluator<ygg::Index<tyr::formalism::Object>>::update(left.added,
                                                                                                  left.removed,
                                                                                                  right.added,
                                                                                                  right.removed,
                                                                                                  workspace);
        }
    };

    template<ConceptOrRoleTag Category>
    struct FromDenotation : QueryValue
    {
        EvaluationIndex<Category> argument;

        FromDenotation(EvaluationIndex<Category> argument, std::span<const ygg::Index<ygg::database::Column>> columns) : QueryValue(columns), argument(argument)
        {
        }
        void set_element(DenotationElementView<Category> element, bool present)
        {
            if constexpr (std::same_as<Category, ConceptTag>)
                this->set(std::array { element.get_index() }, present);
            else
                this->set(std::array { element.first.get_index(), element.second.get_index() }, present);
        }
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
        {
            this->clear();
            for (const auto element : graph.result(argument).indices())
            {
                if constexpr (std::same_as<Category, ConceptTag>)
                    this->set(std::array { element }, true);
                else
                    this->set(std::array { element.first, element.second }, true);
            }
            this->clear_delta();
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            this->clear_delta();
            const auto& delta = graph.change(argument);
            for (const auto& element : delta.removed)
                set_element(element, false);
            for (const auto& element : delta.added)
                set_element(element, true);
        }
    };

    template<typename Tag>
    struct SetCombination : QueryValue
    {
        QueryEvaluationIndex lhs;
        QueryEvaluationIndex rhs;

        SetCombination(QueryEvaluationIndex lhs, QueryEvaluationIndex rhs, std::span<const ygg::Index<ygg::database::Column>> columns) :
            QueryValue(columns),
            lhs(lhs),
            rhs(rhs)
        {
        }
        template<ygg::database::RelationViewConcept<ygg::Index<tyr::formalism::Object>> Rows>
        void update_rows(EvaluationGraph<Family, Kind>& graph, const Rows& rows)
        {
            const auto& left = graph.result(lhs);
            const auto& right = graph.result(rhs);
            for (size_t i = 0; i < rows.size(); ++i)
            {
                const auto row = rows.row(i);
                const auto present = std::same_as<Tag, QueryUnionTag> ? left.contains(row) || right.contains(row) : left.contains(row) && !right.contains(row);
                // Children hold their final contents: a row mentioned by both
                // deltas must not produce a temporary removal/addition pair.
                this->set(row, present);
            }
        }
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
        {
            this->clear();
            update_rows(graph, graph.result(lhs));
            if constexpr (std::same_as<Tag, QueryUnionTag>)
                update_rows(graph, graph.result(rhs));
            this->clear_delta();
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            this->clear_delta();
            const auto& left = graph.change(lhs);
            const auto& right = graph.change(rhs);
            update_rows(graph, left.added);
            update_rows(graph, left.removed);
            update_rows(graph, right.added);
            update_rows(graph, right.removed);
        }
    };

    template<typename Tag>
    struct Selection : QueryValue
    {
        QueryEvaluationIndex argument;
        FamilyQueryView<Family, Tag> expression;

        Selection(QueryEvaluationIndex argument, FamilyQueryView<Family, Tag> expression) :
            QueryValue(expression.get_schema().span()),
            argument(argument),
            expression(expression)
        {
        }
        template<ygg::database::RelationViewConcept<ygg::Index<tyr::formalism::Object>> Rows>
        void update_rows(const Rows& rows, bool present)
        {
            for (size_t i = 0; i < rows.size(); ++i)
            {
                const auto row = rows.row(i);
                if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
                {
                    if (row[expression.get_data().lhs_position] != row[expression.get_data().rhs_position])
                        continue;
                }
                else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
                {
                    if (row[expression.get_data().position] != expression.get_object().get_index())
                        continue;
                }
                this->set(row, present);
            }
        }
        template<StateEvaluationContextConcept<Family, Kind> Context>
        void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
        {
            this->clear();
            update_rows(graph.result(argument), true);
            this->clear_delta();
        }
        void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
        {
            this->clear_delta();
            const auto& delta = graph.change(argument);
            update_rows(delta.removed, false);
            update_rows(delta.added, true);
        }
    };

    using Operation = std::variant<Static,
                                   Atomic<tyr::formalism::FluentTag>,
                                   Atomic<tyr::formalism::DerivedTag>,
                                   Projection,
                                   Join,
                                   FromDenotation<ConceptTag>,
                                   FromDenotation<RoleTag>,
                                   SetCombination<QueryUnionTag>,
                                   SetCombination<QueryDifferenceTag>,
                                   Selection<QueryRenameTag>,
                                   Selection<QuerySelectEqualTag>,
                                   Selection<QuerySelectValueTag>>;

    FamilyQueryView<Family> m_expression;
    Operation m_operation;
    static Operation prepare_operation(FamilyQueryView<Family> expression, EvaluationGraph<Family, Kind>& graph);
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
void QueryNode<Family, Kind>::QueryValue::set(std::span<const ygg::Index<tyr::formalism::Object>> row, bool present)
{
    const auto position = m_result.find(row);
    if (position.has_value() == present)
        return;
    if (present)
    {
        m_delta.added.insert(row);
        m_result.insert(row);
    }
    else
    {
        m_delta.removed.insert(row);
        m_result.erase(*position);
    }
}

template<FamilyTag Family, tyr::TaskKind Kind>
auto QueryNode<Family, Kind>::prepare_operation(FamilyQueryView<Family> expression, EvaluationGraph<Family, Kind>& graph) -> Operation
{
    if (expression.is_static())
        return Static(expression);
    return ygg::visit(
        [&]<typename Tag>(FamilyQueryView<Family, Tag> concrete) -> Operation
        {
            if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::FluentTag>>)
                return Atomic<tyr::formalism::FluentTag>(concrete);
            else if constexpr (std::same_as<Tag, AtomicStateTag<tyr::formalism::DerivedTag>>)
                return Atomic<tyr::formalism::DerivedTag>(concrete);
            else if constexpr (std::same_as<Tag, QueryProjectTag>)
                return Projection(graph.prepare(concrete.get_arg()), concrete.get_data().plan);
            else if constexpr (std::same_as<Tag, QueryJoinTag>)
                return Join(graph.prepare(concrete.get_lhs()), graph.prepare(concrete.get_rhs()), concrete.get_data().plan);
            else if constexpr (std::same_as<Tag, QueryConceptTag>)
                return FromDenotation<ConceptTag>(graph.prepare(concrete.get_arg()), concrete.get_schema().span());
            else if constexpr (std::same_as<Tag, QueryRoleTag>)
                return FromDenotation<RoleTag>(graph.prepare(concrete.get_arg()), concrete.get_schema().span());
            else if constexpr (std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
                return SetCombination<Tag>(graph.prepare(concrete.get_lhs()), graph.prepare(concrete.get_rhs()), concrete.get_schema().span());
            else if constexpr (std::same_as<Tag, QueryRenameTag> || std::same_as<Tag, QuerySelectEqualTag> || std::same_as<Tag, QuerySelectValueTag>)
                return Selection<Tag>(graph.prepare(concrete.get_arg()), concrete);
            else
                throw std::logic_error("Incremental query: a static constructor was marked dynamic.");
        },
        expression.get_variant());
}

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
