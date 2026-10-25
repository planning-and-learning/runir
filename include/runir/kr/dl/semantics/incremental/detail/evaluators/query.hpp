#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_QUERY_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_QUERY_HPP_

#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/atomic_query.hpp"
#include "runir/kr/dl/semantics/query_evaluation.hpp"

#include <concepts>
#include <cstddef>
#include <ranges>
#include <span>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/database/semantics/incremental/generic_join.hpp>
#include <yggdrasil/database/semantics/incremental/query.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

using ObjectIndex = ygg::Index<tyr::formalism::Object>;

/// A maintained query result with the last update's exact delta.
struct QueryValue : ygg::database::incremental::detail::QueryValue
{
    using ygg::database::incremental::detail::QueryValue::QueryValue;
    auto& clear() noexcept
    {
        result.clear();
        delta.clear();
        return result;
    }
};

template<tyr::TaskKind Kind>
struct QueryStaticEvaluator : QueryValue
{
    QueryView<> expression;
    bool initialized = false;

    explicit QueryStaticEvaluator(QueryView<> expression_) : QueryValue(expression_.get_schema().span()), expression(expression_) {}
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>&, Context& context)
    {
        if (!initialized)
        {
            auto& result_rows = this->clear();
            auto output = context.for_result(true);
            ygg::visit([&](auto concrete) { semantics::detail::evaluate_query<Kind>(concrete, output, result_rows); }, expression.get_variant());
            initialized = true;
        }
        this->delta.clear();
    }
    void update(EvaluationGraph<Kind>&, const Delta&, ygg::database::Workspace&) {}
};

template<tyr::TaskKind Kind, tyr::formalism::FactKind Fact>
struct QueryAtomicEvaluator : AtomicQueryEvaluator<Fact>
{
    explicit QueryAtomicEvaluator(QueryView<AtomicStateTag<Fact>> expression) : AtomicQueryEvaluator<Fact>(expression) {}
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>&, Context& context)
    { AtomicQueryEvaluator<Fact>::template initialize<Kind>(context.get_state()); }
    void update(EvaluationGraph<Kind>&, const Delta& delta, ygg::database::Workspace&)
    {
        if constexpr (std::same_as<Fact, tyr::formalism::FluentTag>)
            AtomicQueryEvaluator<Fact>::update(delta.added.fluent_atoms, delta.removed.fluent_atoms);
        else
            AtomicQueryEvaluator<Fact>::update(delta.added.derived_atoms, delta.removed.derived_atoms);
    }
};

template<tyr::TaskKind Kind>
struct QueryProjectionEvaluator : ygg::database::incremental::ProjectionEvaluator
{
    QueryEvaluationIndex argument;

    QueryProjectionEvaluator(QueryEvaluationIndex argument_, const ygg::database::ProjectionPlan& plan) :
        ygg::database::incremental::ProjectionEvaluator(plan),
        argument(argument_)
    {
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context& context)
    { ygg::database::incremental::ProjectionEvaluator::initialize(graph.result(argument), context.get_workspace().get_database_workspace()); }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace& workspace)
    { ygg::database::incremental::ProjectionEvaluator::update(graph.change(argument).change(), workspace); }
};

template<tyr::TaskKind Kind>
struct QueryJoinEvaluator : ygg::database::incremental::JoinEvaluator
{
    QueryEvaluationIndex lhs;
    QueryEvaluationIndex rhs;

    QueryJoinEvaluator(QueryEvaluationIndex lhs_, QueryEvaluationIndex rhs_, const ygg::database::JoinPlan& plan) :
        ygg::database::incremental::JoinEvaluator(plan),
        lhs(lhs_),
        rhs(rhs_)
    {
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context& context)
    { ygg::database::incremental::JoinEvaluator::initialize(graph.result(lhs), graph.result(rhs), context.get_workspace().get_database_workspace()); }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace& workspace)
    { ygg::database::incremental::JoinEvaluator::update(graph.change(lhs).change(), graph.change(rhs).change(), workspace); }
};

template<tyr::TaskKind Kind>
struct QueryGenericJoinEvaluator : ygg::database::incremental::GenericJoinEvaluator
{
    std::vector<QueryEvaluationIndex> args;

    QueryGenericJoinEvaluator(std::vector<QueryEvaluationIndex> args_, const ygg::database::GenericJoinPlan& plan) :
        ygg::database::incremental::GenericJoinEvaluator(plan),
        args(std::move(args_))
    {
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    {
        ygg::database::incremental::GenericJoinEvaluator::initialize(
            args | std::views::transform([&](QueryEvaluationIndex arg) -> const auto& { return graph.result(arg); }));
    }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&)
    {
        ygg::database::incremental::GenericJoinEvaluator::update(args
                                                                 | std::views::transform([&](QueryEvaluationIndex arg) { return graph.change(arg).change(); }));
    }
};

template<tyr::TaskKind Kind, ConceptOrRoleTag Category>
struct QueryFromDenotationEvaluator : QueryValue
{
    EvaluationIndex<Category> argument;

    QueryFromDenotationEvaluator(EvaluationIndex<Category> argument_, std::span<const ygg::database::ColumnLayout> columns) :
        QueryValue(columns),
        argument(argument_)
    {
    }
    void set_element(DenotationElementView<Category> element, bool present)
    {
        const auto columns = this->result.columns().span();
        if constexpr (std::same_as<Category, ConceptTag>)
            this->set(ygg::database::encode_row(std::tuple { element.get_index().get_value() }, columns), present);
        else
            this->set(ygg::database::encode_row(std::tuple { element.first.get_index().get_value(), element.second.get_index().get_value() }, columns),
                      present);
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    { semantics::detail::insert_denotation_rows<Category>(graph.result(argument), this->clear()); }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&)
    {
        this->delta.clear();
        const auto& child_delta = graph.change(argument);
        for (const auto& element : child_delta.removed)
            set_element(element, false);
        for (const auto& element : child_delta.added)
            set_element(element, true);
    }
};

template<tyr::TaskKind Kind, typename Tag>
struct QuerySetCombinationEvaluator : QueryValue
{
    QueryEvaluationIndex lhs;
    QueryEvaluationIndex rhs;

    QuerySetCombinationEvaluator(QueryEvaluationIndex lhs_, QueryEvaluationIndex rhs_, std::span<const ygg::database::ColumnLayout> columns) :
        QueryValue(columns),
        lhs(lhs_),
        rhs(rhs_)
    {
    }
    template<ygg::database::RelationViewConcept Rows, std::invocable<std::span<const std::byte>, bool> Emit>
    void evaluate_rows(EvaluationGraph<Kind>& graph, const Rows& rows, Emit emit)
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
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
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
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&)
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

template<tyr::TaskKind Kind, typename Tag>
struct QuerySelectionEvaluator : QueryValue
{
    QueryEvaluationIndex argument;
    QueryView<Tag> expression;

    QuerySelectionEvaluator(QueryEvaluationIndex argument_, QueryView<Tag> expression_) :
        QueryValue(expression_.get_schema().span()),
        argument(argument_),
        expression(expression_)
    {
    }
    template<ygg::database::RelationViewConcept Rows, std::invocable<std::span<const std::byte>> Emit>
    void evaluate_rows(const Rows& rows, Emit emit)
    {
        for (size_t i = 0; i < rows.size(); ++i)
        {
            const auto row = rows.row(i);
            const auto typed = ygg::database::Row(row, rows.columns().span());
            if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
            {
                if (ObjectIndex(typed.template get<ygg::uint_t>(expression.get_data().lhs_position))
                    != ObjectIndex(typed.template get<ygg::uint_t>(expression.get_data().rhs_position)))
                    continue;
            }
            else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
            {
                if (ObjectIndex(typed.template get<ygg::uint_t>(expression.get_data().position)) != expression.get_data().object)
                    continue;
            }
            emit(row);
        }
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    {
        auto& output = this->clear();
        evaluate_rows(graph.result(argument), [&output](auto row) { output.insert(row); });
    }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&)
    {
        this->delta.clear();
        const auto& child_delta = graph.change(argument);
        evaluate_rows(child_delta.removed, [this](auto row) { this->set(row, false); });
        evaluate_rows(child_delta.added, [this](auto row) { this->set(row, true); });
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
