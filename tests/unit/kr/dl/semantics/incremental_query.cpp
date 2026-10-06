#include "planning_fixtures.hpp"

#include <algorithm>
#include <concepts>
#include <filesystem>
#include <gtest/gtest.h>
#include <iterator>
#include <ranges>
#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/dl/semantics/incremental/delta.hpp>
#include <runir/kr/dl/semantics/incremental/detail/atomic_query.hpp>
#include <runir/kr/dl/semantics/incremental/evaluation.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <string>
#include <type_traits>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <vector>
#include <yggdrasil/database/incremental/projection.hpp>

namespace runir::tests
{
namespace
{
namespace dl = kr::dl;
namespace sem = dl::semantics;
namespace db = ygg::database;
using Ext = kr::ExtFamilyTag;
using Fluent = tyr::formalism::FluentTag;
using Derived = tyr::formalism::DerivedTag;
using Object = ygg::Index<tyr::formalism::Object>;

auto parse_query(const std::string& expression, tyr::formalism::planning::DomainView domain, dl::ConstructorRepositoryFor<Ext>& repository)
{
    const auto count = kr::ps::ext::dl::parse_numerical("(n_count " + expression + ")", domain, repository);
    return count.get_variant().template get<ygg::Index<dl::Numerical<Ext, dl::CountTag>>>().get_arg().template get<ygg::Index<dl::Query<Ext>>>();
}

auto rows(const auto& relation)
{
    auto result = std::vector<std::vector<Object>> {};
    for (size_t i = 0; i < relation.size(); ++i)
    {
        const auto row = relation.row(i);
        result.emplace_back(row.begin(), row.end());
    }
    std::ranges::sort(result);
    return result;
}

auto difference(const auto& lhs, const auto& rhs)
{
    auto result = std::remove_cvref_t<decltype(lhs)> {};
    std::ranges::set_difference(lhs, rhs, std::back_inserter(result));
    return result;
}

void expect_net_delta(const auto& evaluator, const auto& before)
{
    const auto after = rows(evaluator.get_result());
    EXPECT_EQ(rows(evaluator.get_delta().added), difference(after, before));
    EXPECT_EQ(rows(evaluator.get_delta().removed), difference(before, after));
}

void expect_empty_delta(const auto& evaluator)
{
    EXPECT_TRUE(evaluator.get_delta().added.empty());
    EXPECT_TRUE(evaluator.get_delta().removed.empty());
}

template<tyr::TaskKind Kind>
void check_atomic_projection()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/dl/query";
    const auto search = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
        else
            return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
    }();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    const auto a = domain.get_constants()[0].get_index();
    const auto first_candidates = search->successor_generator->get_labeled_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator);
    const auto first = std::ranges::find_if(first_candidates, [&](const auto& candidate) { return candidate.label.get_objects()[0].get_index() == a; });
    ASSERT_NE(first, first_candidates.end());
    const auto second_candidates = search->successor_generator->get_labeled_successor_nodes(first->node, *search->state_repository, *search->axiom_evaluator);
    const auto second = std::ranges::find_if(second_candidates, [&](const auto& candidate) { return candidate.label.get_objects()[0].get_index() == a; });
    ASSERT_NE(second, second_candidates.end());

    auto constructors = dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    const auto triple = parse_query(R"((q_atomic_state "triple" (x y z)))", domain, *constructors);
    const auto copied = parse_query(R"((q_atomic_state "copied" (x y z)))", domain, *constructors);
    const auto ready = parse_query(R"((q_atomic_state "ready" ()))", domain, *constructors);
    const auto projected_triple = parse_query(R"((q_project (x) (q_atomic_state "triple" (x y z))))", domain, *constructors);
    const auto projected_copied = parse_query(R"((q_project (x) (q_atomic_state "copied" (x y z))))", domain, *constructors);
    auto triple_leaf =
        sem::incremental::detail::AtomicQueryEvaluator<Fluent>(triple.get_variant().template get<ygg::Index<dl::Query<Ext, dl::AtomicStateTag<Fluent>>>>());
    auto copied_leaf =
        sem::incremental::detail::AtomicQueryEvaluator<Derived>(copied.get_variant().template get<ygg::Index<dl::Query<Ext, dl::AtomicStateTag<Derived>>>>());
    auto ready_leaf =
        sem::incremental::detail::AtomicQueryEvaluator<Fluent>(ready.get_variant().template get<ygg::Index<dl::Query<Ext, dl::AtomicStateTag<Fluent>>>>());
    auto triple_projection = db::incremental::ProjectionEvaluator<Object>(
        projected_triple.get_variant().template get<ygg::Index<dl::Query<Ext, dl::QueryProjectTag>>>().get_data().plan);
    auto copied_projection = db::incremental::ProjectionEvaluator<Object>(
        projected_copied.get_variant().template get<ygg::Index<dl::Query<Ext, dl::QueryProjectTag>>>().get_data().plan);
    auto workspace = db::Workspace<Object> {};
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<Ext>(denotations);
    auto builder = sem::Builder {};
    auto register_data = ygg::Data<sem::RegisterValues> {};
    const auto registers = ygg::make_view(register_data, *search->task->get_repository());
    auto argument_data = ygg::Data<sem::CallArguments> {};
    const auto arguments = denotations.insert(argument_data).first;
    const auto publications = [&] { return storage.get_denotation_repository(false).get_relation_repository().size(); };
    const auto compare_full = [&](auto state)
    {
        storage.reset_dynamic();
        auto context =
            sem::StateEvaluationContext<Ext, Kind, decltype(state), std::remove_cvref_t<decltype(registers)>>(state, builder, storage, arguments, registers);
        EXPECT_EQ(rows(triple_leaf.get_result()), rows(sem::evaluate<Kind>(triple, context)));
        EXPECT_EQ(rows(copied_leaf.get_result()), rows(sem::evaluate<Kind>(copied, context)));
        EXPECT_EQ(rows(ready_leaf.get_result()), rows(sem::evaluate<Kind>(ready, context)));
        EXPECT_EQ(rows(triple_projection.get_result()), rows(sem::evaluate<Kind>(projected_triple, context)));
        EXPECT_EQ(rows(copied_projection.get_result()), rows(sem::evaluate<Kind>(projected_copied, context)));
    };
    const auto initialize = [&](auto state)
    {
        const auto published = publications();
        triple_leaf.template initialize<Kind>(state);
        copied_leaf.template initialize<Kind>(state);
        ready_leaf.template initialize<Kind>(state);
        triple_projection.initialize(triple_leaf.get_result(), workspace);
        copied_projection.initialize(copied_leaf.get_result(), workspace);
        EXPECT_EQ(publications(), published);
        expect_empty_delta(triple_leaf);
        expect_empty_delta(copied_leaf);
        expect_empty_delta(ready_leaf);
        expect_empty_delta(triple_projection);
        expect_empty_delta(copied_projection);
        compare_full(state);
    };
    const auto update = [&](const auto& delta, auto state)
    {
        const auto before_triple = rows(triple_leaf.get_result());
        const auto before_copied = rows(copied_leaf.get_result());
        const auto before_ready = rows(ready_leaf.get_result());
        const auto before_triple_projection = rows(triple_projection.get_result());
        const auto before_copied_projection = rows(copied_projection.get_result());
        const auto published = publications();
        triple_leaf.update(delta.added.fluent_atoms, delta.removed.fluent_atoms);
        copied_leaf.update(delta.added.derived_atoms, delta.removed.derived_atoms);
        ready_leaf.update(delta.added.fluent_atoms, delta.removed.fluent_atoms);
        triple_projection.update(triple_leaf.get_delta().added, triple_leaf.get_delta().removed, workspace);
        copied_projection.update(copied_leaf.get_delta().added, copied_leaf.get_delta().removed, workspace);
        EXPECT_EQ(publications(), published);
        expect_net_delta(triple_leaf, before_triple);
        expect_net_delta(copied_leaf, before_copied);
        expect_net_delta(ready_leaf, before_ready);
        expect_net_delta(triple_projection, before_triple_projection);
        expect_net_delta(copied_projection, before_copied_projection);
        compare_full(state);
    };

    EXPECT_THROW(triple_leaf.update({}, {}), std::logic_error);
    initialize(initial.get_state());
    ASSERT_EQ(triple_leaf.get_result().size(), 4);
    ASSERT_EQ(triple_projection.get_result().size(), 2);
    ASSERT_TRUE(ready_leaf.get_result().contains({}));
    auto first_delta = sem::incremental::Delta<Ext> {};
    first_delta.template assign<Kind>(initial.get_state(), registers, first->node.get_state(), registers);
    EXPECT_THROW(triple_leaf.update(first_delta.removed.fluent_atoms, {}), std::invalid_argument);
    EXPECT_THROW(triple_leaf.update({}, {}), std::logic_error);
    initialize(initial.get_state());
    update(first_delta, first->node.get_state());
    // The first action removes both ready and a triple; each leaf filters the other predicate.
    EXPECT_EQ(triple_leaf.get_delta().removed.size(), 1);
    EXPECT_EQ(ready_leaf.get_delta().removed.size(), 1);
    EXPECT_TRUE(ready_leaf.get_delta().removed.contains({}));
    EXPECT_TRUE(triple_projection.get_result().contains({ a }));
    expect_empty_delta(triple_projection);
    expect_empty_delta(copied_projection);

    auto second_delta = sem::incremental::Delta<Ext> {};
    second_delta.template assign<Kind>(first->node.get_state(), registers, second->node.get_state(), registers);
    update(second_delta, second->node.get_state());
    EXPECT_FALSE(triple_projection.get_result().contains({ a }));
    EXPECT_TRUE(triple_projection.get_delta().removed.contains({ a }));
    EXPECT_TRUE(copied_projection.get_delta().removed.contains({ a }));
    expect_empty_delta(ready_leaf);

    second_delta.reverse();
    update(second_delta, first->node.get_state());
    EXPECT_TRUE(triple_projection.get_delta().added.contains({ a }));
    first_delta.reverse();
    update(first_delta, initial.get_state());
    expect_empty_delta(triple_projection);
    expect_empty_delta(copied_projection);
    EXPECT_TRUE(ready_leaf.get_delta().added.contains({}));

    // Reinitialization replaces the baseline and clears every previously emitted delta.
    const auto borrowed_builder = second->node.get_state().get_state_builder();
    initialize(ygg::make_view(borrowed_builder, *search->task));
    EXPECT_EQ(triple_leaf.get_result().size(), 2);
    EXPECT_FALSE(triple_projection.get_result().contains({ a }));
    EXPECT_TRUE(ready_leaf.get_result().empty());
    EXPECT_TRUE(denotations.get_relation_repository().empty());
}

template<tyr::TaskKind Kind>
void check_query_graph()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/dl/query";
    const auto make_search = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
        else
            return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
    };
    const auto search = make_search();
    auto nodes = std::vector<tyr::planning::Node<Kind>> { search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator) };
    for (size_t i = 0; i < 4; ++i)
    {
        const auto successors = search->successor_generator->get_labeled_successor_nodes(nodes.back(), *search->state_repository, *search->axiom_evaluator);
        ASSERT_FALSE(successors.empty());
        nodes.push_back(successors.front().node);
    }

    auto constructors = dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    const auto domain = search->task->get_domain().get_domain();
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<Ext>(denotations);
    auto builder = sem::Builder {};
    auto register_data = ygg::Data<sem::RegisterValues> {};
    const auto registers = ygg::make_view(register_data, *search->task->get_repository());
    auto argument_data = ygg::Data<sem::CallArguments> {};
    const auto arguments = denotations.insert(argument_data).first;
    const auto context_for = [&](auto state) {
        return sem::StateEvaluationContext<Ext, Kind, decltype(state), std::remove_cvref_t<decltype(registers)>>(state, builder, storage, arguments, registers);
    };
    auto deltas = std::vector<sem::incremental::Delta<Ext>>(4);
    for (size_t i = 0; i < deltas.size(); ++i)
        deltas[i].template assign<Kind>(nodes[i].get_state(), registers, nodes[i + 1].get_state(), registers);
    auto workspace = db::Workspace<Object> {};
    const auto empty_delta = sem::incremental::Delta<Ext> {};
    const auto triple = std::string(R"((q_atomic_state "triple" (x y z)))");
    const auto copied = std::string(R"((q_atomic_state "copied" (x y z)))");
    const auto fixed = std::string(R"((q_atomic_state "fixed" (x y z)))");
    const auto expressions = std::vector<std::string> {
        triple,
        "(q_project () (q_project (x) " + triple + "))",
        "(q_join " + triple + " " + fixed + ")",
        "(q_join " + fixed + " " + triple + ")",
        "(q_join " + triple + " " + copied + ")",
        // Repeated subexpressions must be updated once, even when both join inputs use them.
        "(q_join " + triple + " " + triple + ")",
        "(q_join (q_project (x) " + triple + ") (q_project (x) " + copied + "))",
        R"((q_atomic_goal "triple" true (x y z)))",
        R"((q_atomic_goal "triple" false (x y z)))",
        "(q_union " + fixed + " " + fixed + ")",
        "(q_join " + triple + " (q_union " + fixed + " " + fixed + "))",
        "(q_union " + triple + " " + fixed + ")",
        "(q_union " + triple + " " + copied + ")",
        "(q_difference " + triple + " " + fixed + ")",
        "(q_difference " + fixed + " " + triple + ")",
        "(q_difference " + triple + " " + copied + ")",
        "(q_rename (u v w) " + triple + ")",
        "(q_select_equal y z " + triple + ")",
        "(q_select_value x \"a\" " + triple + ")",
    };
    for (const auto& expression : expressions)
    {
        SCOPED_TRACE(expression);
        const auto query = parse_query(expression, domain, *constructors);
        auto evaluator = sem::incremental::QueryEvaluator<Ext, Kind>(*search->task, query);
        EXPECT_THROW(evaluator.update(empty_delta, workspace), std::logic_error);
        const auto expect_unpublished = [&]
        {
            EXPECT_TRUE(storage.get_caches().get_queries(false).empty());
            EXPECT_TRUE(storage.get_denotation_repository(false).get_relation_repository().empty());
        };
        const auto compare_full = [&](auto state)
        {
            auto context = context_for(state);
            EXPECT_EQ(rows(evaluator.get_result()), rows(sem::evaluate<Kind>(query, context)));
        };
        const auto update = [&](const auto& delta, const auto& state)
        {
            const auto before = rows(evaluator.get_result());
            storage.reset_dynamic();
            evaluator.update(delta, workspace);
            expect_unpublished();
            expect_net_delta(evaluator, before);
            compare_full(state);

            const auto after = rows(evaluator.get_result());
            evaluator.update(empty_delta, workspace);
            expect_empty_delta(evaluator);
            EXPECT_EQ(rows(evaluator.get_result()), after);
        };

        storage.reset_all();
        auto initial_context = context_for(nodes.front().get_state());
        evaluator.initialize(initial_context);
        expect_unpublished();
        expect_empty_delta(evaluator);
        compare_full(nodes.front().get_state());
        for (size_t i = 0; i < deltas.size(); ++i)
            update(deltas[i], nodes[i + 1].get_state());
        for (size_t i = deltas.size(); i > 0; --i)
        {
            auto reversed = deltas[i - 1];
            reversed.reverse();
            update(reversed, nodes[i - 1].get_state());
        }

        // Static rows are owned by the evaluator, not the caches used to initialize them.
        const auto before_reset = rows(evaluator.get_result());
        storage.reset_all();
        EXPECT_EQ(rows(evaluator.get_result()), before_reset);
        const auto borrowed_builder = nodes.back().get_state().get_state_builder();
        auto final_context = context_for(ygg::make_view(borrowed_builder, *search->task));
        evaluator.initialize(final_context);
        expect_unpublished();
        EXPECT_TRUE(storage.get_caches().get_queries(true).empty());
        EXPECT_TRUE(storage.get_denotation_repository(true).get_relation_repository().empty());
        expect_empty_delta(evaluator);
        compare_full(final_context.get_state());
    }

    const auto goal = parse_query(R"((q_atomic_goal "triple" true (x y z)))", domain, *constructors);
    auto evaluator = sem::incremental::QueryEvaluator<Ext, Kind>(*search->task, goal);
    auto initial_context = context_for(nodes.front().get_state());
    evaluator.initialize(initial_context);
    const auto other = make_search();
    const auto other_initial = other->successor_generator->get_initial_node(*other->state_repository, *other->axiom_evaluator);
    auto other_denotations = sem::DenotationRepositoryFactory().create(other->task->get_repository());
    auto other_storage = sem::EvaluationStorage<Ext>(other_denotations);
    const auto other_registers = ygg::make_view(register_data, *other->task->get_repository());
    auto other_argument_data = ygg::Data<sem::CallArguments> {};
    const auto other_arguments = other_denotations.insert(other_argument_data).first;
    auto other_context =
        sem::StateEvaluationContext<Ext, Kind, tyr::planning::StateView<Kind>, std::remove_cvref_t<decltype(other_registers)>>(other_initial.get_state(),
                                                                                                                               builder,
                                                                                                                               other_storage,
                                                                                                                               other_arguments,
                                                                                                                               other_registers);
    EXPECT_THROW(evaluator.initialize(other_context), std::invalid_argument);
}
}  // namespace

TEST(RunirKrDlIncrementalQuery, GroundAtomicProjectionMatchesFullEvaluation) { check_atomic_projection<tyr::GroundTag>(); }
TEST(RunirKrDlIncrementalQuery, LiftedAtomicProjectionMatchesFullEvaluation) { check_atomic_projection<tyr::LiftedTag>(); }
TEST(RunirKrDlIncrementalQuery, GroundQueryGraphMatchesFullEvaluation) { check_query_graph<tyr::GroundTag>(); }
TEST(RunirKrDlIncrementalQuery, LiftedQueryGraphMatchesFullEvaluation) { check_query_graph<tyr::LiftedTag>(); }

}  // namespace runir::tests
