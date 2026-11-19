#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_QUERY_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_QUERY_HPP_

#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/atomic_query.hpp"
#include "runir/kr/dl/semantics/query_evaluation.hpp"

#include <array>
#include <concepts>
#include <cstddef>
#include <ranges>
#include <span>
#include <tuple>
#include <tyr/datalog/lifted/database/semantics/incremental/generic_join.hpp>
#include <tyr/datalog/lifted/database/semantics/incremental/query.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>

namespace runir::kr::dl::semantics::incremental::detail
{

using ObjectIndex = urd::index<tyr::formalism::Object>;

/// A maintained query result with the last update's exact delta.
struct QueryValue : tyr::datalog::database::incremental::detail::QueryValue
{
    using tyr::datalog::database::incremental::detail::QueryValue::QueryValue;

    ygg::FreeScratchRelation& clear() noexcept
    {
        result->clear();
        delta.clear();
        return *result;
    }
};

template<tyr::TaskKind Kind>
struct QueryStaticEvaluator : QueryValue
{
    QueryView<> expression;
    bool initialized = false;

    QueryStaticEvaluator(ygg::FreeScratchRelations& scratch_, QueryView<> expression_) :
        QueryValue(scratch_, schema_of(expression_)),
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

    void update(EvaluationGraph<Kind>&, const Delta&, tyr::datalog::database::Workspace&) {}
};

template<tyr::TaskKind Kind, tyr::formalism::FactKind Fact>
struct QueryAtomicEvaluator : AtomicQueryEvaluator<Fact>
{
    QueryAtomicEvaluator(ygg::FreeScratchRelations& scratch, QueryView<AtomicStateTag<Fact>> expression) : AtomicQueryEvaluator<Fact>(scratch, expression)
    {
    }

    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>&, Context& context)
    { AtomicQueryEvaluator<Fact>::template initialize<Kind>(context.get_state()); }

    void update(EvaluationGraph<Kind>&, const Delta& delta, tyr::datalog::database::Workspace&)
    {
        if constexpr (std::same_as<Fact, tyr::formalism::FluentTag>)
            AtomicQueryEvaluator<Fact>::update(delta.added.fluent_atoms, delta.removed.fluent_atoms);
        else
            AtomicQueryEvaluator<Fact>::update(delta.added.derived_atoms, delta.removed.derived_atoms);
    }
};

template<tyr::TaskKind Kind>
struct QueryProjectionEvaluator : tyr::datalog::database::incremental::ProjectionEvaluator
{
    QueryEvaluationIndex argument;

    QueryProjectionEvaluator(ygg::FreeScratchRelations& scratch, QueryEvaluationIndex argument_, const tyr::datalog::database::ProjectionPlan& plan) :
        tyr::datalog::database::incremental::ProjectionEvaluator(scratch, plan),
        argument(argument_)
    {
    }

    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context& context)
    { tyr::datalog::database::incremental::ProjectionEvaluator::initialize(graph.result(argument), context.get_workspace().get_database_workspace()); }

    void update(EvaluationGraph<Kind>& graph, const Delta&, tyr::datalog::database::Workspace& workspace)
    { tyr::datalog::database::incremental::ProjectionEvaluator::update(graph.change(argument).change(), workspace); }
};

template<tyr::TaskKind Kind>
struct QueryJoinEvaluator : tyr::datalog::database::incremental::JoinEvaluator
{
    QueryEvaluationIndex lhs;
    QueryEvaluationIndex rhs;

    QueryJoinEvaluator(ygg::FreeScratchRelations& scratch,
                       QueryEvaluationIndex lhs_,
                       QueryEvaluationIndex rhs_,
                       const tyr::datalog::database::JoinPlan& plan) :
        tyr::datalog::database::incremental::JoinEvaluator(scratch, plan),
        lhs(lhs_),
        rhs(rhs_)
    {
    }

    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context& context)
    { tyr::datalog::database::incremental::JoinEvaluator::initialize(graph.result(lhs), graph.result(rhs), context.get_workspace().get_database_workspace()); }

    void update(EvaluationGraph<Kind>& graph, const Delta&, tyr::datalog::database::Workspace& workspace)
    { tyr::datalog::database::incremental::JoinEvaluator::update(graph.change(lhs).change(), graph.change(rhs).change(), workspace); }
};

template<tyr::TaskKind Kind>
struct QueryGenericJoinEvaluator : tyr::datalog::database::incremental::GenericJoinEvaluator
{
    urd::Store<urd::sequence<QueryEvaluationIndex>> args;

    QueryGenericJoinEvaluator(ygg::FreeScratchRelations& scratch,
                              urd::Store<urd::sequence<QueryEvaluationIndex>> args_,
                              const tyr::datalog::database::GenericJoinPlan& plan) :
        tyr::datalog::database::incremental::GenericJoinEvaluator(scratch, plan),
        args(std::move(args_))
    {
    }

    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    {
        tyr::datalog::database::incremental::GenericJoinEvaluator::initialize(
            args | std::views::transform([&](QueryEvaluationIndex arg) { return graph.result(arg); }));
    }

    void update(EvaluationGraph<Kind>& graph, const Delta&, tyr::datalog::database::Workspace&)
    {
        tyr::datalog::database::incremental::GenericJoinEvaluator::update(
            args | std::views::transform([&](QueryEvaluationIndex arg) { return graph.change(arg).change(); }));
    }
};

template<tyr::TaskKind Kind, ConceptOrRoleTag Category>
struct QueryFromDenotationEvaluator : QueryValue
{
    EvaluationIndex<Category> argument;

    QueryFromDenotationEvaluator(ygg::FreeScratchRelations& scratch_, EvaluationIndex<Category> argument_, std::span<const ygg::ColumnLayout> columns) :
        QueryValue(scratch_, columns),
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

    void update(EvaluationGraph<Kind>& graph, const Delta&, tyr::datalog::database::Workspace&)
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

    QuerySetCombinationEvaluator(ygg::FreeScratchRelations& scratch_,
                                 QueryEvaluationIndex lhs_,
                                 QueryEvaluationIndex rhs_,
                                 std::span<const ygg::ColumnLayout> columns) :
        QueryValue(scratch_, columns),
        lhs(lhs_),
        rhs(rhs_)
    {
    }

    template<ygg::RelationViewConcept<ygg::Schema> Rows, std::invocable<decltype(std::declval<const Rows&>().row(0)), bool> Emit>
    void evaluate_rows(EvaluationGraph<Kind>& graph, const Rows& rows, Emit emit)
    {
        const auto left_operand = graph.result(lhs);
        const auto right_operand = graph.result(rhs);
        for (size_t i = 0; i < rows.size(); ++i)
        {
            const auto row = rows.row(i);
            const auto present = ygg::with_values(row,
                                                  [&](const auto& values)
                                                  {
                                                      return std::same_as<Tag, QueryUnionTag> ?
                                                                 left_operand.contains(values) || right_operand.contains(values) :
                                                                 left_operand.contains(values) && !right_operand.contains(values);
                                                  });
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
                ygg::with_values(row, [&output](const auto& values) { output.insert(values); });
        };
        evaluate_rows(graph, graph.result(lhs), emit);
        if constexpr (std::same_as<Tag, QueryUnionTag>)
            evaluate_rows(graph, graph.result(rhs), emit);
    }

    void update(EvaluationGraph<Kind>& graph, const Delta&, tyr::datalog::database::Workspace&)
    {
        this->delta.clear();
        const auto [left_added, left_removed] = graph.change(lhs).change();
        const auto [right_added, right_removed] = graph.change(rhs).change();
        const auto emit = [this](auto row, bool present) { ygg::with_values(row, [this, present](const auto& values) { this->set(values, present); }); };
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

    QuerySelectionEvaluator(ygg::FreeScratchRelations& scratch_, QueryEvaluationIndex argument_, QueryView<Tag> expression_) :
        QueryValue(scratch_, expression_.get_schema()),
        argument(argument_),
        expression(expression_)
    {
    }

    template<ygg::RelationViewConcept<ygg::Schema> Rows, std::invocable<decltype(std::declval<const Rows&>().row(0))> Emit>
    void evaluate_rows(const Rows& rows, Emit emit)
    {
        for (size_t i = 0; i < rows.size(); ++i)
        {
            const auto row = rows.row(i);
            if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
            {
                const auto& plan = expression.get_plan();
                if (ygg::value_of(row, plan.left_position()) != ygg::value_of(row, plan.right_position()))
                    continue;
            }
            else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
            {
                const auto& plan = expression.get_plan();
                if (ygg::value_of(row, plan.position()) != plan.value())
                    continue;
            }
            emit(row);
        }
    }

    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    {
        auto& output = this->clear();
        evaluate_rows(graph.result(argument), [&output](auto row) { ygg::with_values(row, [&output](const auto& values) { output.insert(values); }); });
    }

    void update(EvaluationGraph<Kind>& graph, const Delta&, tyr::datalog::database::Workspace&)
    {
        this->delta.clear();
        const auto [added, removed] = graph.change(argument).change();
        evaluate_rows(removed, [this](auto row) { ygg::with_values(row, [this](const auto& values) { this->set(values, false); }); });
        evaluate_rows(added, [this](auto row) { ygg::with_values(row, [this](const auto& values) { this->set(values, true); }); });
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
