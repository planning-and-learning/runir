#include "incremental_root_evaluators.hpp"
#include "planning_fixtures.hpp"

#include <algorithm>
#include <concepts>
#include <filesystem>
#include <gtest/gtest.h>
#include <iterator>
#include <ranges>
#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/incremental/delta.hpp>
#include <runir/kr/dl/semantics/incremental/detail/atomic_query.hpp>
#include <runir/kr/dl/semantics/incremental/evaluation.hpp>
#include <runir/kr/dl/semantics/state_evaluation.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <string>
#include <tuple>
#include <type_traits>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <vector>
#include <yggdrasil/database/operators/relational/incremental/projection.hpp>

namespace runir::tests
{
namespace
{
using Fluent = tyr::formalism::FluentTag;
using Derived = tyr::formalism::DerivedTag;
using Object = ygg::Index<tyr::formalism::Object>;

auto parse_query(const std::string& expression, tyr::formalism::planning::DomainView domain, kr::dl::ConstructorRepository& repository)
{
    const auto count = kr::ps::ext::dl::parse_numerical("(n_count " + expression + ")", domain, repository);
    return count.get_variant().template get<ygg::Index<kr::dl::Numerical<kr::dl::CountTag>>>().get_arg().template get<ygg::Index<kr::dl::Query<>>>();
}

auto rows(const auto& relation)
{
    auto result = std::vector<std::vector<Object>> {};
    for (size_t i = 0; i < relation.size(); ++i)
    {
        const auto row = ygg::database::Row(relation.row(i), relation.columns().span());
        auto& values = result.emplace_back();
        for (size_t j = 0; j < row.size(); ++j)
            values.emplace_back(row.get<ygg::uint_t>(j));
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

void expect_net_delta(auto& evaluator, const auto& before)
{
    const auto after = rows(evaluator.get_result());
    EXPECT_EQ(rows(evaluator.get_delta().added), difference(after, before));
    EXPECT_EQ(rows(evaluator.get_delta().removed), difference(before, after));
}

void expect_empty_delta(auto& evaluator)
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

    auto constructors = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    const auto triple = parse_query(R"((q_atomic_state "triple" (x y z)))", domain, *constructors);
    const auto copied = parse_query(R"((q_atomic_state "copied" (x y z)))", domain, *constructors);
    const auto ready = parse_query(R"((q_atomic_state "ready" ()))", domain, *constructors);
    const auto projected_triple = parse_query(R"((q_project (x) (q_atomic_state "triple" (x y z))))", domain, *constructors);
    const auto projected_copied = parse_query(R"((q_project (x) (q_atomic_state "copied" (x y z))))", domain, *constructors);
    auto triple_leaf = kr::dl::semantics::incremental::detail::AtomicQueryEvaluator<Fluent>(
        triple.get_variant().template get<ygg::Index<kr::dl::Query<kr::dl::AtomicStateTag<Fluent>>>>());
    auto copied_leaf = kr::dl::semantics::incremental::detail::AtomicQueryEvaluator<Derived>(
        copied.get_variant().template get<ygg::Index<kr::dl::Query<kr::dl::AtomicStateTag<Derived>>>>());
    auto ready_leaf = kr::dl::semantics::incremental::detail::AtomicQueryEvaluator<Fluent>(
        ready.get_variant().template get<ygg::Index<kr::dl::Query<kr::dl::AtomicStateTag<Fluent>>>>());
    auto triple_projection = ygg::database::incremental::ProjectionEvaluator(
        projected_triple.get_variant().template get<ygg::Index<kr::dl::Query<kr::dl::QueryProjectTag>>>().get_data().plan);
    auto copied_projection = ygg::database::incremental::ProjectionEvaluator(
        projected_copied.get_variant().template get<ygg::Index<kr::dl::Query<kr::dl::QueryProjectTag>>>().get_data().plan);
    auto workspace = ygg::database::Workspace {};
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto builder = kr::dl::semantics::Builder {};
    auto register_data = ygg::Data<kr::dl::semantics::RegisterValues> {};
    const auto registers = ygg::make_view(register_data, *search->task->get_repository());
    auto argument_data = ygg::Data<kr::dl::semantics::CallArguments> {};
    const auto arguments = denotations.insert(argument_data).first;
    const auto publications = [&] { return storage.get_denotation_repository(false).get_relation_repository().size(); };
    const auto compare_full = [&](auto state)
    {
        storage.reset_dynamic();
        auto context = kr::dl::semantics::StateEvaluationContext<Kind, decltype(state)>(state, builder, storage, arguments, registers);
        EXPECT_EQ(rows(triple_leaf.get_result()), rows(kr::dl::semantics::evaluate<Kind>(triple, context)));
        EXPECT_EQ(rows(copied_leaf.get_result()), rows(kr::dl::semantics::evaluate<Kind>(copied, context)));
        EXPECT_EQ(rows(ready_leaf.get_result()), rows(kr::dl::semantics::evaluate<Kind>(ready, context)));
        EXPECT_EQ(rows(triple_projection.get_result()), rows(kr::dl::semantics::evaluate<Kind>(projected_triple, context)));
        EXPECT_EQ(rows(copied_projection.get_result()), rows(kr::dl::semantics::evaluate<Kind>(projected_copied, context)));
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
        triple_projection.update(triple_leaf.get_delta().change(), workspace);
        copied_projection.update(copied_leaf.get_delta().change(), workspace);
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
    ASSERT_TRUE(ready_leaf.get_result().contains(std::tuple<> {}));
    auto first_delta = kr::dl::semantics::incremental::Delta {};
    first_delta.template assign<Kind>(initial.get_state(), registers, first->node.get_state(), registers);
    EXPECT_THROW(triple_leaf.update(first_delta.removed.fluent_atoms, {}), std::invalid_argument);
    EXPECT_THROW(triple_leaf.update({}, {}), std::logic_error);
    initialize(initial.get_state());
    update(first_delta, first->node.get_state());
    // The first action removes both ready and a triple; each leaf filters the other predicate.
    EXPECT_EQ(triple_leaf.get_delta().removed.size(), 1);
    EXPECT_EQ(ready_leaf.get_delta().removed.size(), 1);
    EXPECT_TRUE(ready_leaf.get_delta().removed.contains(std::tuple<> {}));
    EXPECT_TRUE(triple_projection.get_result().contains(std::tuple { a.get_value() }));
    expect_empty_delta(triple_projection);
    expect_empty_delta(copied_projection);

    auto second_delta = kr::dl::semantics::incremental::Delta {};
    second_delta.template assign<Kind>(first->node.get_state(), registers, second->node.get_state(), registers);
    update(second_delta, second->node.get_state());
    EXPECT_FALSE(triple_projection.get_result().contains(std::tuple { a.get_value() }));
    EXPECT_TRUE(triple_projection.get_delta().removed.contains(std::tuple { a.get_value() }));
    EXPECT_TRUE(copied_projection.get_delta().removed.contains(std::tuple { a.get_value() }));
    expect_empty_delta(ready_leaf);

    second_delta.reverse();
    update(second_delta, first->node.get_state());
    EXPECT_TRUE(triple_projection.get_delta().added.contains(std::tuple { a.get_value() }));
    first_delta.reverse();
    update(first_delta, initial.get_state());
    expect_empty_delta(triple_projection);
    expect_empty_delta(copied_projection);
    EXPECT_TRUE(ready_leaf.get_delta().added.contains(std::tuple<> {}));

    // Reinitialization replaces the baseline and clears every previously emitted delta.
    const auto borrowed_builder = second->node.get_state().get_state_builder();
    initialize(ygg::make_view(borrowed_builder, *search->task));
    EXPECT_EQ(triple_leaf.get_result().size(), 2);
    EXPECT_FALSE(triple_projection.get_result().contains(std::tuple { a.get_value() }));
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

    auto constructors = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    const auto domain = search->task->get_domain().get_domain();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto builder = kr::dl::semantics::Builder {};
    auto register_data = ygg::Data<kr::dl::semantics::RegisterValues> {};
    const auto registers = ygg::make_view(register_data, *search->task->get_repository());
    auto argument_data = ygg::Data<kr::dl::semantics::CallArguments> {};
    const auto arguments = denotations.insert(argument_data).first;
    const auto context_for = [&](auto state)
    { return kr::dl::semantics::StateEvaluationContext<Kind, decltype(state)>(state, builder, storage, arguments, registers); };
    auto deltas = std::vector<kr::dl::semantics::incremental::Delta>(4);
    for (size_t i = 0; i < deltas.size(); ++i)
        deltas[i].template assign<Kind>(nodes[i].get_state(), registers, nodes[i + 1].get_state(), registers);
    auto workspace = ygg::database::Workspace {};
    const auto empty_delta = kr::dl::semantics::incremental::Delta {};
    const auto triple = std::string(R"((q_atomic_state "triple" (x y z)))");
    const auto copied = std::string(R"((q_atomic_state "copied" (x y z)))");
    const auto fixed = std::string(R"((q_atomic_state "fixed" (x y z)))");
    const auto static_role = std::string(R"((q_role (x y) (r_complement (r_identity (c_top)))))");
    const auto expressions = std::vector<std::string> {
        triple,
        fixed,
        static_role,
        R"((q_concept x (c_top)))",
        "(q_project () (q_project (x) " + triple + "))",
        "(q_join " + triple + " " + fixed + ")",
        "(q_join " + fixed + " " + triple + ")",
        "(q_join " + triple + " " + copied + ")",
        "(q_generic_join (x y z) (z y x) " + triple + " " + fixed + ")",
        "(q_generic_join (z x y) (x y z) " + triple + " " + copied + " " + R"((q_concept x (c_top))))",
        "(q_join " + triple + " (q_empty (y)))",
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
        "(q_rename (u v w) " + fixed + ")",
        "(q_select_equal y z " + triple + ")",
        "(q_select_value x \"a\" " + triple + ")",
        "(q_concept x (c_project x " + triple + "))",
        "(q_role (x y) (r_project x y " + triple + "))",
        "(q_must_pass (x) (q_project (x y) " + triple + ") " + triple + " (q_project (x z) " + triple + R"() (q_concept p (c_top))))",
        "(q_must_pass (x) (q_project (x y) " + triple + ") " + copied + " (q_project (x z) " + fixed + ") (q_project (p) (q_rename (p q r) " + triple + ")))",
        // Sources, targets and probes may use closures; only the edges are search-free.
        "(q_must_pass (x) (q_project (x y) " + triple + ") " + triple + " (q_project (x z) " + triple
            + R"() (q_concept p (c_some (r_reflexive_transitive_closure (r_project y z (q_atomic_state "triple" (x y z)))) (c_top)))))",
    };
    for (const auto& expression : expressions)
    {
        SCOPED_TRACE(expression);
        const auto query = parse_query(expression, domain, *constructors);
        auto evaluator = QueryRootEvaluator<Kind>(*search->task, query);
        const auto expect_unpublished = [&]
        {
            EXPECT_TRUE(storage.get_caches().get_queries(false).empty());
            EXPECT_TRUE(storage.get_denotation_repository(false).get_relation_repository().empty());
        };
        const auto compare_full = [&](auto& evaluated, auto state)
        {
            auto context = context_for(state);
            EXPECT_EQ(rows(evaluated.get_result()), rows(kr::dl::semantics::evaluate<Kind>(query, context)));
        };
        // Updates from the previous state, whose rows are `before`, and returns the rows of the reached state.
        const auto update = [&](const auto& delta, const auto& state, const auto& before)
        {
            storage.reset_dynamic();
            auto context = context_for(state);
            auto updated = evaluator.update(context, delta, workspace);
            expect_unpublished();
            expect_net_delta(updated, before);  // Throws unless updated from the delta.
            compare_full(updated, state);

            auto after = rows(updated.get_result());
            auto unchanged = evaluator.update(context, empty_delta, workspace);
            expect_empty_delta(unchanged);
            EXPECT_EQ(rows(unchanged.get_result()), after);
            return after;
        };

        storage.reset_all();
        auto initial_context = context_for(nodes.front().get_state());
        // A fresh evaluator has no previous state: an update initializes it, so it has no delta.
        auto fresh = evaluator.update(initial_context, empty_delta, workspace);
        EXPECT_THROW(fresh.get_delta(), std::logic_error);
        auto initial = evaluator.initialize(initial_context);
        expect_unpublished();
        EXPECT_TRUE(std::ranges::equal(initial.get_result().columns().span(), query.get_schema().span()));
        if (expression == static_role)
        {
            // Its feature child may be interned; the static query root is owned here.
            EXPECT_TRUE(storage.get_caches().get_queries(true).empty());
            EXPECT_TRUE(storage.get_denotation_repository(true).get_relation_repository().empty());
        }
        EXPECT_THROW(initial.get_delta(), std::logic_error);  // Initialized, not updated from a delta.
        compare_full(initial, nodes.front().get_state());
        auto current = rows(initial.get_result());
        for (size_t i = 0; i < deltas.size(); ++i)
            current = update(deltas[i], nodes[i + 1].get_state(), current);
        for (size_t i = deltas.size(); i > 0; --i)
        {
            auto reversed = deltas[i - 1];
            reversed.reverse();
            current = update(reversed, nodes[i - 1].get_state(), current);
        }

        // Static rows are owned by the evaluator, not the caches used to initialize them.
        auto last = evaluator.update(initial_context, empty_delta, workspace);
        const auto& retained = last.get_result();
        const auto before_reset = rows(retained);
        EXPECT_EQ(before_reset, current);
        storage.reset_all();
        EXPECT_EQ(rows(retained), before_reset);
        const auto borrowed_builder = nodes.back().get_state().get_state_builder();
        auto final_context = context_for(ygg::make_view(borrowed_builder, *search->task));
        auto final_state = evaluator.initialize(final_context);
        expect_unpublished();
        EXPECT_TRUE(storage.get_caches().get_queries(true).empty());
        EXPECT_TRUE(storage.get_denotation_repository(true).get_relation_repository().empty());
        EXPECT_THROW(final_state.get_delta(), std::logic_error);  // Initialized, not updated from a delta.
        compare_full(final_state, final_context.get_state());
    }

    const auto goal = parse_query(R"((q_atomic_goal "triple" true (x y z)))", domain, *constructors);
    auto evaluator = QueryRootEvaluator<Kind>(*search->task, goal);
    auto initial_context = context_for(nodes.front().get_state());
    evaluator.initialize(initial_context);
    const auto other = make_search();
    const auto other_initial = other->successor_generator->get_initial_node(*other->state_repository, *other->axiom_evaluator);
    auto other_denotations = kr::dl::semantics::DenotationRepositoryFactory().create(other->task->get_repository());
    auto other_storage = kr::dl::semantics::EvaluationStorage(other_denotations);
    const auto other_registers = ygg::make_view(register_data, *other->task->get_repository());
    auto other_argument_data = ygg::Data<kr::dl::semantics::CallArguments> {};
    const auto other_arguments = other_denotations.insert(other_argument_data).first;
    auto other_context = kr::dl::semantics::StateEvaluationContext<Kind, tyr::planning::StateView<Kind>>(other_initial.get_state(),
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
