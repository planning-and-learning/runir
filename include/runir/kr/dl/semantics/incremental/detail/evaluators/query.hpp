#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_QUERY_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_QUERY_HPP_

#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/atomic_query.hpp"

#include <array>
#include <concepts>
#include <span>

namespace runir::kr::dl::semantics::incremental::detail
{

class QueryValue
{
    ygg::Builder<ygg::database::Relation<ygg::Index<tyr::formalism::Object>>> m_result;
    ygg::database::incremental::Delta<ygg::Index<tyr::formalism::Object>> m_delta;

protected:
    explicit QueryValue(std::span<const ygg::Index<ygg::database::Column>> columns) : m_result(columns), m_delta(columns) {}
    auto& clear() noexcept
    {
        m_result.clear();
        m_delta.clear();
        return m_result;
    }
    void clear_delta() noexcept { m_delta.clear(); }
    template<ygg::database::RelationViewConcept<ygg::Index<tyr::formalism::Object>> Source>
    void assign(const Source& source)
    {
        ygg::database::assign(m_result, source);
        m_delta.clear();
    }
    void set(std::span<const ygg::Index<tyr::formalism::Object>> row, bool present)
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

public:
    const auto& get_result() const noexcept { return m_result; }
    const auto& get_delta() const noexcept { return m_delta; }
};

template<FamilyTag Family, tyr::TaskKind Kind>
struct QueryStaticEvaluator : QueryValue
{
    FamilyQueryView<Family> expression;
    bool initialized = false;

    explicit QueryStaticEvaluator(FamilyQueryView<Family> expression) : QueryValue(expression.get_schema().span()), expression(expression) {}
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

template<FamilyTag Family, tyr::TaskKind Kind, tyr::formalism::FactKind Fact>
struct QueryAtomicEvaluator : AtomicQueryEvaluator<Fact>
{
    explicit QueryAtomicEvaluator(FamilyQueryView<Family, AtomicStateTag<Fact>> expression) : AtomicQueryEvaluator<Fact>(expression) {}
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

template<FamilyTag Family, tyr::TaskKind Kind>
struct QueryProjectionEvaluator : ygg::database::incremental::ProjectionEvaluator<ygg::Index<tyr::formalism::Object>>
{
    QueryEvaluationIndex argument;

    QueryProjectionEvaluator(QueryEvaluationIndex argument, const ygg::database::ProjectionPlan& plan) :
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

template<FamilyTag Family, tyr::TaskKind Kind>
struct QueryJoinEvaluator : ygg::database::incremental::JoinEvaluator<ygg::Index<tyr::formalism::Object>>
{
    QueryEvaluationIndex lhs;
    QueryEvaluationIndex rhs;

    QueryJoinEvaluator(QueryEvaluationIndex lhs, QueryEvaluationIndex rhs, const ygg::database::JoinPlan& plan) :
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
        ygg::database::incremental::JoinEvaluator<ygg::Index<tyr::formalism::Object>>::update(left.added, left.removed, right.added, right.removed, workspace);
    }
};

template<FamilyTag Family, tyr::TaskKind Kind, ConceptOrRoleTag Category>
struct QueryFromDenotationEvaluator : QueryValue
{
    EvaluationIndex<Category> argument;

    QueryFromDenotationEvaluator(EvaluationIndex<Category> argument, std::span<const ygg::Index<ygg::database::Column>> columns) :
        QueryValue(columns),
        argument(argument)
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
        auto& output = this->clear();
        for (const auto element : graph.result(argument).indices())
        {
            if constexpr (std::same_as<Category, ConceptTag>)
                output.insert(std::array { element });
            else
                output.insert(std::array { element.first, element.second });
        }
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

template<FamilyTag Family, tyr::TaskKind Kind, typename Tag>
struct QuerySetCombinationEvaluator : QueryValue
{
    QueryEvaluationIndex lhs;
    QueryEvaluationIndex rhs;

    QuerySetCombinationEvaluator(QueryEvaluationIndex lhs, QueryEvaluationIndex rhs, std::span<const ygg::Index<ygg::database::Column>> columns) :
        QueryValue(columns),
        lhs(lhs),
        rhs(rhs)
    {
    }
    template<ygg::database::RelationViewConcept<ygg::Index<tyr::formalism::Object>> Rows,
             std::invocable<std::span<const ygg::Index<tyr::formalism::Object>>, bool> Emit>
    void evaluate_rows(EvaluationGraph<Family, Kind>& graph, const Rows& rows, Emit emit)
    {
        const auto& left = graph.result(lhs);
        const auto& right = graph.result(rhs);
        for (size_t i = 0; i < rows.size(); ++i)
        {
            const auto row = rows.row(i);
            const auto present = std::same_as<Tag, QueryUnionTag> ? left.contains(row) || right.contains(row) : left.contains(row) && !right.contains(row);
            // Children hold their final contents: a row mentioned by both
            // deltas must not produce a temporary removal/addition pair.
            emit(row, present);
        }
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        auto& output = this->clear();
        const auto emit = [&output](auto row, bool present)
        {
            if (present)
                output.insert(row);
        };
        evaluate_rows(graph, graph.result(lhs), emit);
        if constexpr (std::same_as<Tag, QueryUnionTag>)
            evaluate_rows(graph, graph.result(rhs), emit);
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
    {
        this->clear_delta();
        const auto& left = graph.change(lhs);
        const auto& right = graph.change(rhs);
        const auto emit = [this](auto row, bool present) { this->set(row, present); };
        evaluate_rows(graph, left.added, emit);
        evaluate_rows(graph, left.removed, emit);
        evaluate_rows(graph, right.added, emit);
        evaluate_rows(graph, right.removed, emit);
    }
};

template<FamilyTag Family, tyr::TaskKind Kind, typename Tag>
struct QuerySelectionEvaluator : QueryValue
{
    QueryEvaluationIndex argument;
    FamilyQueryView<Family, Tag> expression;

    QuerySelectionEvaluator(QueryEvaluationIndex argument, FamilyQueryView<Family, Tag> expression) :
        QueryValue(expression.get_schema().span()),
        argument(argument),
        expression(expression)
    {
    }
    template<ygg::database::RelationViewConcept<ygg::Index<tyr::formalism::Object>> Rows,
             std::invocable<std::span<const ygg::Index<tyr::formalism::Object>>> Emit>
    void evaluate_rows(const Rows& rows, Emit emit)
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
            emit(row);
        }
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        auto& output = this->clear();
        evaluate_rows(graph.result(argument), [&output](auto row) { output.insert(row); });
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
    {
        this->clear_delta();
        const auto& delta = graph.change(argument);
        evaluate_rows(delta.removed, [this](auto row) { this->set(row, false); });
        evaluate_rows(delta.added, [this](auto row) { this->set(row, true); });
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
