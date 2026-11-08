#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_QUERY_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_QUERY_HPP_

#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/atomic_query.hpp"
#include "runir/kr/dl/semantics/query_evaluation.hpp"

#include <concepts>
#include <cstddef>
#include <ranges>
#include <array>
#include <span>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/database/semantics/incremental/generic_join.hpp>
#include <yggdrasil/database/semantics/incremental/query.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

using ObjectIndex = urd::index<tyr::formalism::Object>;

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

    QueryStaticEvaluator(const ygg::database::Repository& layer_, QueryView<> expression_) :
        QueryValue(layer_, expression_.get_schema()),
        expression(expression_)
    {
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>&, Context& context)
    {
        if (!initialized)
        {
            auto& result_rows = this->clear();
            auto output = context.for_result(true);
            std::visit([&](auto concrete) { semantics::detail::evaluate_query<Kind>(concrete, output, result_rows); }, expression.get_variant());
            initialized = true;
        }
        this->delta.clear();
    }
    void update(EvaluationGraph<Kind>&, const Delta&, ygg::database::Workspace&) {}
};

template<tyr::TaskKind Kind, tyr::formalism::FactKind Fact>
struct QueryAtomicEvaluator : AtomicQueryEvaluator<Fact>
{
    QueryAtomicEvaluator(const ygg::database::Repository& layer, QueryView<AtomicStateTag<Fact>> expression) : AtomicQueryEvaluator<Fact>(layer, expression) {}
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

    QueryProjectionEvaluator(const ygg::database::Repository& layer, QueryEvaluationIndex argument_, const ygg::database::ProjectionPlan& plan) :
        ygg::database::incremental::ProjectionEvaluator(layer, plan),
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

    QueryJoinEvaluator(const ygg::database::Repository& layer, QueryEvaluationIndex lhs_, QueryEvaluationIndex rhs_, const ygg::database::JoinPlan& plan) :
        ygg::database::incremental::JoinEvaluator(layer, plan),
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
    urd::Store<urd::sequence<QueryEvaluationIndex>> args;

    QueryGenericJoinEvaluator(const ygg::database::Repository& layer,
                              urd::Store<urd::sequence<QueryEvaluationIndex>> args_,
                              const ygg::database::GenericJoinPlan& plan) :
        ygg::database::incremental::GenericJoinEvaluator(layer, plan),
        args(std::move(args_))
    {
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    {
        ygg::database::incremental::GenericJoinEvaluator::initialize(
            args | std::views::transform([&](QueryEvaluationIndex arg) { return graph.result(arg); }));
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

    QueryFromDenotationEvaluator(const ygg::database::Repository& layer_, EvaluationIndex<Category> argument_, std::span<const ygg::ColumnLayout> columns) :
        QueryValue(layer_, columns),
        argument(argument_)
    {
    }
    void set_element(DenotationElementView<Category> element, bool present)
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            this->set(std::array<ygg::Value, 1> { ygg::encode_value<ygg::uint_t>(element.index().value) }, present);
        else
            this->set(std::array<ygg::Value, 2> { ygg::encode_value<ygg::uint_t>(element.first.index().value),
                                                  ygg::encode_value<ygg::uint_t>(element.second.index().value) },
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

    QuerySetCombinationEvaluator(const ygg::database::Repository& layer_,
                                 QueryEvaluationIndex lhs_,
                                 QueryEvaluationIndex rhs_,
                                 std::span<const ygg::ColumnLayout> columns) :
        QueryValue(layer_, columns),
        lhs(lhs_),
        rhs(rhs_)
    {
    }
    template<ygg::database::RelationViewConcept Rows, std::invocable<ygg::database::RowPool::row_type, bool> Emit>
    void evaluate_rows(EvaluationGraph<Kind>& graph, const Rows& rows, Emit emit)
    {
        const auto left = graph.result(lhs);
        const auto right = graph.result(rhs);
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
        const auto [left_added, left_removed] = graph.change(lhs).change();
        const auto [right_added, right_removed] = graph.change(rhs).change();
        const auto emit = [this](auto row, bool present) { this->set(row, present); };
        evaluate_rows(graph, left_added, emit);
        evaluate_rows(graph, left_removed, emit);
        evaluate_rows(graph, right_added, emit);
        evaluate_rows(graph, right_removed, emit);
    }
};

template<tyr::TaskKind Kind, typename Tag>
struct QuerySelectionEvaluator : QueryValue
{
    QueryEvaluationIndex argument;
    QueryView<Tag> expression;

    QuerySelectionEvaluator(const ygg::database::Repository& layer_, QueryEvaluationIndex argument_, QueryView<Tag> expression_) :
        QueryValue(layer_, expression_.get_schema()),
        argument(argument_),
        expression(expression_)
    {
    }
    template<ygg::database::RelationViewConcept Rows, std::invocable<ygg::database::RowPool::row_type> Emit>
    void evaluate_rows(const Rows& rows, Emit emit)
    {
        for (size_t i = 0; i < rows.size(); ++i)
        {
            const auto row = rows.row(i);
            if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
            {
                const auto& plan = expression.data().plan;
                if (row.get(plan.lhs_position()) != row.get(plan.rhs_position()))
                    continue;
            }
            else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
            {
                const auto& plan = expression.data().plan;
                if (row.get(plan.position()) != plan.value())
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
        const auto [added, removed] = graph.change(argument).change();
        evaluate_rows(removed, [this](auto row) { this->set(row, false); });
        evaluate_rows(added, [this](auto row) { this->set(row, true); });
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
