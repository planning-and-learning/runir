#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_QUERY_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_QUERY_HPP_

#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/atomic_query.hpp"

#include <concepts>
#include <cstddef>
#include <span>
#include <tuple>
#include <yggdrasil/database/semantics/incremental/query.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

using ObjectIndex = ygg::Index<tyr::formalism::Object>;

/// A maintained query result with the last update's exact delta.
struct QueryValue : ygg::database::incremental::detail::QueryValue<ObjectValues>
{
    using ygg::database::incremental::detail::QueryValue<ObjectValues>::QueryValue;
    auto& clear() noexcept
    {
        result.clear();
        delta.clear();
        return result;
    }
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
            auto& result = this->clear();
            auto output = context.for_result(true);
            ygg::visit([&](auto concrete) { semantics::detail::evaluate_query<Kind>(concrete, output, result); }, expression.get_variant());
            initialized = true;
        }
        this->delta.clear();
    }
    void update(EvaluationGraph<Family, Kind>&, const Delta<Family>&, ygg::database::Workspace<ObjectValues>&) {}
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
    void update(EvaluationGraph<Family, Kind>&, const Delta<Family>& delta, ygg::database::Workspace<ObjectValues>&)
    {
        if constexpr (std::same_as<Fact, tyr::formalism::FluentTag>)
            AtomicQueryEvaluator<Fact>::update(delta.added.fluent_atoms, delta.removed.fluent_atoms);
        else
            AtomicQueryEvaluator<Fact>::update(delta.added.derived_atoms, delta.removed.derived_atoms);
    }
};

template<FamilyTag Family, tyr::TaskKind Kind>
struct QueryProjectionEvaluator : ygg::database::incremental::ProjectionEvaluator<ObjectValues>
{
    QueryEvaluationIndex argument;

    QueryProjectionEvaluator(QueryEvaluationIndex argument, const ygg::database::ProjectionPlan<ObjectValues>& plan) :
        ygg::database::incremental::ProjectionEvaluator<ObjectValues>(plan),
        argument(argument)
    {
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
    {
        ygg::database::incremental::ProjectionEvaluator<ObjectValues>::initialize(graph.result(argument),
                                                                                                        context.get_workspace().get_database_workspace());
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ObjectValues>& workspace)
    {
        ygg::database::incremental::ProjectionEvaluator<ObjectValues>::update(graph.change(argument).change(), workspace);
    }
};

template<FamilyTag Family, tyr::TaskKind Kind>
struct QueryJoinEvaluator : ygg::database::incremental::JoinEvaluator<ObjectValues>
{
    QueryEvaluationIndex lhs;
    QueryEvaluationIndex rhs;

    QueryJoinEvaluator(QueryEvaluationIndex lhs, QueryEvaluationIndex rhs, const ygg::database::JoinPlan<ObjectValues>& plan) :
        ygg::database::incremental::JoinEvaluator<ObjectValues>(plan),
        lhs(lhs),
        rhs(rhs)
    {
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
    {
        ygg::database::incremental::JoinEvaluator<ObjectValues>::initialize(graph.result(lhs),
                                                                                                  graph.result(rhs),
                                                                                                  context.get_workspace().get_database_workspace());
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ObjectValues>& workspace)
    {
        ygg::database::incremental::JoinEvaluator<ObjectValues>::update(graph.change(lhs).change(), graph.change(rhs).change(), workspace);
    }
};

template<FamilyTag Family, tyr::TaskKind Kind, ConceptOrRoleTag Category>
struct QueryFromDenotationEvaluator : QueryValue
{
    EvaluationIndex<Category> argument;

    QueryFromDenotationEvaluator(EvaluationIndex<Category> argument, std::span<const ygg::database::ColumnLayout> columns) :
        QueryValue(columns),
        argument(argument)
    {
    }
    void set_element(DenotationElementView<Category> element, bool present)
    {
        const auto columns = this->result.columns().span();
        if constexpr (std::same_as<Category, ConceptTag>)
            this->set(ygg::database::encode_row<ObjectValues>(std::tuple { element.get_index() }, columns), present);
        else
            this->set(ygg::database::encode_row<ObjectValues>(std::tuple { element.first.get_index(), element.second.get_index() }, columns), present);
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        auto& output = this->clear();
        for (const auto element : graph.result(argument).indices())
        {
            if constexpr (std::same_as<Category, ConceptTag>)
                output.insert(std::tuple { element });
            else
                output.insert(std::tuple { element.first, element.second });
        }
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ObjectValues>&)
    {
        this->delta.clear();
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

    QuerySetCombinationEvaluator(QueryEvaluationIndex lhs, QueryEvaluationIndex rhs, std::span<const ygg::database::ColumnLayout> columns) :
        QueryValue(columns),
        lhs(lhs),
        rhs(rhs)
    {
    }
    template<ygg::database::RelationViewConcept<ObjectValues> Rows,
             std::invocable<std::span<const std::byte>, bool> Emit>
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
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ObjectValues>&)
    {
        this->delta.clear();
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
    template<ygg::database::RelationViewConcept<ObjectValues> Rows,
             std::invocable<std::span<const std::byte>> Emit>
    void evaluate_rows(const Rows& rows, Emit emit)
    {
        for (size_t i = 0; i < rows.size(); ++i)
        {
            const auto row = rows.row(i);
            const auto typed = ygg::database::Row<ObjectValues>(row, rows.columns().span());
            if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
            {
                if (typed.get<ObjectIndex>(expression.get_data().lhs_position) != typed.get<ObjectIndex>(expression.get_data().rhs_position))
                    continue;
            }
            else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
            {
                if (typed.get<ObjectIndex>(expression.get_data().position) != expression.get_data().object)
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
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ObjectValues>&)
    {
        this->delta.clear();
        const auto& delta = graph.change(argument);
        evaluate_rows(delta.removed, [this](auto row) { this->set(row, false); });
        evaluate_rows(delta.added, [this](auto row) { this->set(row, true); });
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
