#include "incremental_root_evaluators.hpp"
#include "planning_fixtures.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <array>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <gtest/gtest.h>
#include <limits>
#include <new>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/incremental/delta.hpp>
#include <runir/kr/dl/semantics/incremental/detail/atomic_query.hpp>
#include <runir/kr/dl/semantics/incremental/evaluation.hpp>
#include <runir/kr/ps/ext/detail/proof_search.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/program_executor.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/task_context.hpp>
#include <string>
#include <tuple>
#include <tyr/datalog/lifted/database/semantics/incremental/projection.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>
#include <urd/testing/allocations.hpp>
#include <utility>
#include <variant>
#include <vector>

// This executable links urd::count_allocations, which replaces the C++ allocation functions only. That covers the
// default std/GTL allocators and urd's containers, including aligned allocations. It does not intercept direct
// malloc/free calls, including Cista column/plan storage. Counting is restricted to the evaluating thread and excludes
// GTest, setup, warmup, and destruction of the retained evaluation objects.

namespace runir::tests
{
using ObjectIndex = urd::index<tyr::formalism::Object>;

TEST(RunirQueries, WarmedExtFeatureEvaluationAllocatesAndFreesNothing)
{
    // Explicit calls check the counter without allocation elision by the compiler.
    const urd::testing::Counted counter_check;
    auto* ordinary = ::operator new(32);
    auto* array = ::operator new[](32);
    auto* aligned = ::operator new(64, std::align_val_t(64));
    auto* aligned_array = ::operator new[](64, std::align_val_t(64));
    ::operator delete(ordinary);
    ::operator delete[](array);
    ::operator delete(aligned, std::align_val_t(64));
    ::operator delete[](aligned_array, std::align_val_t(64));
    const auto checked = urd::testing::counts();
    ASSERT_EQ(checked.allocations, 4);
    ASSERT_EQ(checked.deallocations, 4);

    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../support/data/kr/dl/query";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto arguments = urd::data<kr::dl::semantics::CallArguments>();
    auto registers = urd::data<kr::dl::semantics::RegisterValues>();
    registers.concept_values.resize(8);
    registers.role_values.resize(8);
    const auto object = search->task->get_domain().get_domain().get_constants()[0];
    registers.concept_values[5] = object.index();
    registers.role_values[7] = ::cista::pair(object.index(), object.index());
    const auto empty_arguments = kr::dl::semantics::insert(denotations, arguments).first;
    const auto register_values = kr::dl::semantics::insert(denotations, registers).first;
    auto context = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(initial.get_state(), builder, storage, empty_arguments, register_values);
    const auto nominal = kr::ps::ext::dl::parse_concept(R"((c_nominal "a"))", search->task->get_domain().get_domain(), *repository);
    auto persistent_memo = kr::dl::semantics::DenotationCaches {};
    auto persistent = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(initial.get_state(),
                                                                                builder,
                                                                                persistent_memo,
                                                                                denotations,
                                                                                storage,
                                                                                empty_arguments,
                                                                                register_values);
    arguments.concept_arguments.push_back(kr::dl::semantics::evaluate<tyr::GroundTag>(nominal, persistent).index());
    const auto argument_values = kr::dl::semantics::insert(denotations, arguments).first;
    const auto expression = kr::ps::ext::dl::parse_numerical(
        R"((n_count (q_rename (source target)
                (q_project (x z)
                    (q_join (q_atomic_state "triple" (x y z)) (q_atomic_state "edge" (x y)))))))",
        search->task->get_domain().get_domain(),
        *repository);

    // Clearing dynamic results measures query recomputation and pooled returns.
    for (size_t i = 0; i < 8; ++i)
    {
        storage.reset_dynamic();
        ASSERT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(expression, context).value(), 3);
    }

    bool valid = true;
    const urd::testing::Counted measured;
    for (size_t i = 0; i < 1000; ++i)
    {
        auto borrowed = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(initial.get_state(), builder, storage, argument_values, register_values);
        auto copied = borrowed;
        valid &= &copied.get_workspace() == &builder.get_workspace();
        valid &= &copied.registers().data() == &register_values.data();
        valid &= &copied.arguments().data() == &argument_values.data();
        valid &= copied.registers().at(kr::dl::RegisterIdentifier<kr::dl::ConceptTag>(5)).value().index() == object.index();
        valid &= copied.registers().at(kr::dl::RegisterIdentifier<kr::dl::RoleTag>(7)).value().second.index() == object.index();
        valid &= copied.arguments().at(kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(0)).value().count() == 1;
        storage.reset_dynamic();
        valid &= kr::dl::semantics::evaluate<tyr::GroundTag>(expression, copied).value() == 3;
    }
    const auto counts = urd::testing::counts();

    EXPECT_TRUE(valid);
    EXPECT_EQ(counts.allocations, 0);
    EXPECT_EQ(counts.deallocations, 0);
}

TEST(RunirQueries, WarmedIncrementalQueryEvaluationAllocatesAndFreesNothing)
{
    using Fluent = tyr::formalism::FluentTag;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../support/data/kr/dl/query";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    const auto count = kr::ps::ext::dl::parse_numerical(R"((n_count (q_project (x) (q_atomic_state "triple" (x y z)))))",
                                                        search->task->get_domain().get_domain(),
                                                        *repository);
    const auto query = std::get<urd::index_view<kr::dl::Query<>, runir::kr::dl::ConstructorRepository>>(
        std::get<urd::index_view<kr::dl::Numerical<kr::dl::SemanticFamily, kr::dl::CountTag>, runir::kr::dl::ConstructorRepository>>(count.get_variant())
            .get_arg());
    const auto projected = std::get<urd::index_view<kr::dl::Query<kr::dl::QueryProjectTag>, runir::kr::dl::ConstructorRepository>>(query.get_variant());
    const auto atomic =
        std::get<urd::index_view<kr::dl::Query<kr::dl::AtomicStateTag<Fluent>>, runir::kr::dl::ConstructorRepository>>(projected.get_arg().get_variant());
    const auto layer = ygg::FreeRelationRepository();
    auto leaf = kr::dl::semantics::incremental::detail::AtomicQueryEvaluator<Fluent>(layer, atomic);
    auto projection = tyr::datalog::database::incremental::ProjectionEvaluator(layer, projected.data().plan);
    auto workspace = tyr::datalog::database::Workspace {};
    leaf.initialize<tyr::GroundTag>(initial.get_state());
    projection.initialize(leaf.get_result(), workspace);
    ASSERT_EQ(leaf.get_result().size(), 4);
    ASSERT_EQ(projection.get_result().size(), 2);

    auto registers = urd::data<kr::dl::semantics::RegisterValues> {};
    const auto register_view = urd::resolve(registers, *search->task->get_repository());
    const auto graph_count = kr::ps::ext::dl::parse_numerical(
        R"((n_count (q_project (x)
            (q_join
                (q_join (q_atomic_state "triple" (x y z)) (q_atomic_state "copied" (x y z)))
                (q_join (q_atomic_state "triple" (x y z)) (q_project () (q_atomic_state "fixed" (x y z))))))))",
        search->task->get_domain().get_domain(),
        *repository);
    const auto graph_query = std::get<urd::index_view<kr::dl::Query<>, runir::kr::dl::ConstructorRepository>>(
        std::get<urd::index_view<kr::dl::Numerical<kr::dl::SemanticFamily, kr::dl::CountTag>, runir::kr::dl::ConstructorRepository>>(graph_count.get_variant())
            .get_arg());
    auto graph = QueryRootEvaluator<tyr::GroundTag>(*search->task, graph_query);
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto builder = kr::dl::semantics::Builder {};
    auto argument_data = urd::data<kr::dl::semantics::CallArguments> {};
    const auto arguments = kr::dl::semantics::insert(denotations, argument_data).first;
    const auto register_values = kr::dl::semantics::insert(denotations, registers).first;
    auto context = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(initial.get_state(), builder, storage, arguments, register_values);
    ASSERT_EQ(graph.initialize(context).get_result().size(), 2);

    // Build completed planning transitions before tracking evaluation allocations.
    auto changes = std::array<kr::dl::semantics::incremental::Delta, 4> {};
    auto projected_sizes = std::array<size_t, 5> { 2 };
    // The context of each state the changes pass through, built before tracking.
    auto contexts = urd::Store<urd::sequence<kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>>> { context };
    auto current = initial;
    for (size_t i = 0; i < changes.size(); ++i)
    {
        const auto successors = search->successor_generator->get_successor_nodes(current, *search->state_repository, *search->axiom_evaluator);
        ASSERT_FALSE(successors.empty());
        const auto& next = successors.front();
        changes[i].assign<tyr::GroundTag>(current.get_state(), register_view, next.get_state(), register_view);
        contexts.emplace_back(next.get_state(), builder, storage, arguments, register_values);
        auto objects = urd::Store<urd::set<ObjectIndex>> {};
        for (const auto atom : next.get_state().get_atoms_view(atomic.get_predicate()))
            objects.insert(atom.get_row().template get<ObjectIndex>(0));
        projected_sizes[i + 1] = objects.size();
        current = next;
    }
    ASSERT_EQ(projected_sizes.back(), 0);

    const auto same_rows = [](const auto& lhs, const auto& rhs)
    {
        if (lhs.size() != rhs.size())
            return false;
        for (size_t i = 0; i < lhs.size(); ++i)
            if (!rhs.contains(lhs.row(i)))
                return false;
        return true;
    };
    const auto apply = [&](const auto& change, size_t target, size_t rows, size_t projected_rows, bool adding)
    {
        const auto previous_size = projection.get_result().size();
        leaf.update(change.added.fluent_atoms, change.removed.fluent_atoms);
        const auto& delta = leaf.get_delta();
        projection.update(delta.change(), workspace);
        auto graph_state = graph.update(contexts[target], change, workspace);
        const auto& projected_delta = projection.get_delta();
        const auto [graph_added, graph_removed] = graph_state.get_delta().change();
        const auto [projected_added, projected_removed] = projected_delta.change();
        return leaf.get_result().size() == rows && delta.added.size() == static_cast<size_t>(adding) && delta.removed.size() == static_cast<size_t>(!adding)
               && projection.get_result().size() == projected_rows && projected_delta.added.size() == (adding ? projected_rows - previous_size : 0)
               && projected_delta.removed.size() == (adding ? 0 : previous_size - projected_rows)
               && same_rows(graph_state.get_result(), projection.get_result()) && same_rows(graph_added, projected_added)
               && same_rows(graph_removed, projected_removed);
    };
    const auto cycle = [&]
    {
        bool valid = true;
        for (size_t i = 0; i < changes.size(); ++i)
            valid &= apply(changes[i], i + 1, changes.size() - i - 1, projected_sizes[i + 1], false);
        for (size_t i = changes.size(); i-- > 0;)
        {
            changes[i].reverse();
            valid &= apply(changes[i], i, changes.size() - i, projected_sizes[i], true);
            changes[i].reverse();
        }
        return valid;
    };

    // Warm erasure headroom, shared children, both join inputs, and first/last projection witnesses.
    for (size_t repeat = 0; repeat < 8; ++repeat)
        ASSERT_TRUE(cycle());
    bool valid = true;
    const urd::testing::Counted measured;
    for (size_t repeat = 0; repeat < 1000; ++repeat)
        valid &= cycle();
    const auto counts = urd::testing::counts();
    EXPECT_TRUE(valid);
    EXPECT_EQ(counts.allocations, 0);
    EXPECT_EQ(counts.deallocations, 0);
}

TEST(RunirQueries, WarmedIncrementalDlEvaluationAllocatesAndFreesNothing)
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../support/data/kr/dl/incremental";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    auto concepts = urd::Store<urd::sequence<RootEvaluator<tyr::GroundTag, kr::dl::ConceptTag>>> {};
    auto roles = urd::Store<urd::sequence<RootEvaluator<tyr::GroundTag, kr::dl::RoleTag>>> {};
    auto numericals = urd::Store<urd::sequence<RootEvaluator<tyr::GroundTag, kr::dl::NumericalTag>>> {};
    auto booleans = urd::Store<urd::sequence<RootEvaluator<tyr::GroundTag, kr::dl::BooleanTag>>> {};
    for (const auto expression : { R"((c_atomic_state "present"))", R"((c_atomic_state "copied-present"))", "(c_register 0)", R"((c_atomic_state "fixed"))" })
        concepts.emplace_back(*search->task, kr::ps::ext::dl::parse_concept(expression, domain, *repository));
    for (const auto expression : { R"((r_atomic_state "edge"))", R"((r_atomic_state "copied-edge"))", "(r_register 0)" })
        roles.emplace_back(*search->task, kr::ps::ext::dl::parse_role(expression, domain, *repository));
    for (const auto expression : { R"((c_atomic_state "present"))", R"((r_atomic_state "edge"))", "(c_register 0)", "(r_register 0)" })
    {
        numericals.emplace_back(*search->task, kr::ps::ext::dl::parse_numerical(std::string("(n_count ") + expression + ")", domain, *repository));
        booleans.emplace_back(*search->task, kr::ps::ext::dl::parse_boolean(std::string("(b_nonempty ") + expression + ")", domain, *repository));
    }

    const auto source = kr::ps::ext::dl::parse_concept("(c_register 0)", domain, *repository);
    const auto edge = kr::ps::ext::dl::parse_role(R"((r_atomic_state "edge"))", domain, *repository);
    const auto target = kr::ps::ext::dl::parse_concept("(c_register 1)", domain, *repository);
    const auto source_count = kr::ps::ext::dl::parse_numerical("(n_count (c_register 0))", domain, *repository);
    const auto edge_count = kr::ps::ext::dl::parse_numerical(R"((n_count (r_atomic_state "edge")))", domain, *repository);
    const auto distance = kr::ps::ext::dl::parse_numerical(R"((n_distance (c_register 0) (r_atomic_state "edge") (c_register 1)))", domain, *repository);
    const auto self_distance = kr::ps::ext::dl::parse_numerical(R"((n_distance (c_register 0) (r_atomic_state "edge") (c_register 0)))", domain, *repository);
    const auto closure = kr::ps::ext::dl::parse_role(R"((r_transitive_closure (r_atomic_state "edge")))", domain, *repository);
    const auto nonempty = kr::ps::ext::dl::parse_boolean(R"((b_nonempty (c_some (r_atomic_state "edge") (c_atomic_state "present"))))", domain, *repository);
    const auto projected = kr::ps::ext::dl::parse_concept(R"((c_project x (q_role (x y) (r_atomic_state "edge"))))", domain, *repository);
    const auto projected_role = kr::ps::ext::dl::parse_role(R"((r_project y x (q_role (x y) (r_atomic_state "edge"))))", domain, *repository);
    const auto query_count = kr::ps::ext::dl::parse_numerical(
        R"((n_count (q_project (x) (q_join (q_role (x y) (r_atomic_state "edge")) (q_concept z (c_atomic_state "present"))))))",
        domain,
        *repository);
    auto roots = urd::Store<urd::sequence<kr::dl::semantics::incremental::EvaluationRoot>> { source,     edge,      target,         source_count,
                                                                                             edge_count, distance,  self_distance,  closure,
                                                                                             nonempty,   projected, projected_role, query_count };
    for (const auto expression : {
             R"((c_and (c_not (c_atomic_state "present")) (c_register 0)))",
             R"((c_at_least 1 (r_atomic_state "edge") (c_atomic_state "present")))",
         })
        roots.emplace_back(kr::ps::ext::dl::parse_concept(expression, domain, *repository));
    for (const auto expression : {
             R"((r_composition (r_atomic_state "edge") (r_atomic_state "edge")))",
             R"((r_complement (r_or (r_atomic_state "edge") (r_register 0))))",
         })
        roots.emplace_back(kr::ps::ext::dl::parse_role(expression, domain, *repository));
    roots.emplace_back(
        kr::ps::ext::dl::parse_numerical(R"((n_add (n_distance (c_register 0) (r_atomic_state "edge") (c_register 1)) (n_const 1)))", domain, *repository));

    auto evaluation = kr::dl::semantics::incremental::IncrementalEvaluation<tyr::GroundTag>(*search->task, roots);
    const auto& graph = evaluation.graph();
    const auto source_id = graph.get_index(source);
    const auto edge_id = graph.get_index(edge);
    const auto source_count_id = graph.get_index(source_count);
    const auto edge_count_id = graph.get_index(edge_count);
    const auto distance_id = graph.get_index(distance);
    const auto self_distance_id = graph.get_index(self_distance);
    const auto closure_id = graph.get_index(closure);
    const auto nonempty_id = graph.get_index(nonempty);
    const auto projected_id = graph.get_index(projected);
    const auto projected_role_id = graph.get_index(projected_role);
    const auto query_count_id = graph.get_index(query_count);

    auto registers = std::array<urd::data<kr::dl::semantics::RegisterValues>, 3> {};
    const auto objects = domain.get_constants();
    ASSERT_GE(objects.size(), 3);
    for (size_t i = 0; i < registers.size(); ++i)
    {
        registers[i].concept_values.resize(2);
        registers[i].role_values.resize(1);
        if (i < 2)
        {
            registers[i].concept_values[0] = objects[static_cast<std::ptrdiff_t>(i)].index();
            registers[i].concept_values[1] = objects[static_cast<std::ptrdiff_t>(2 - i)].index();
            registers[i].role_values[0] = ::cista::pair(objects[static_cast<std::ptrdiff_t>(i)].index(), objects[static_cast<std::ptrdiff_t>(i + 1)].index());
        }
    }
    auto changes = std::array<kr::dl::semantics::incremental::Delta, 2> {};
    auto current = initial;
    auto states = std::vector { initial.get_state() };
    for (size_t i = 0; i < changes.size(); ++i)
    {
        const auto successors = search->successor_generator->get_successor_nodes(current, *search->state_repository, *search->axiom_evaluator);
        ASSERT_FALSE(successors.empty());
        const auto& next = successors.front();
        changes[i].assign<tyr::GroundTag>(current.get_state(),
                                          urd::resolve(registers[i], *search->task->get_repository()),
                                          next.get_state(),
                                          urd::resolve(registers[i + 1], *search->task->get_repository()));
        current = next;
        states.push_back(next.get_state());
    }

    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto builder = kr::dl::semantics::Builder {};
    auto argument_data = urd::data<kr::dl::semantics::CallArguments> {};
    const auto arguments = kr::dl::semantics::insert(denotations, argument_data).first;
    const auto context_for = [&](size_t i)
    {
        return kr::dl::semantics::StateEvaluationContext<tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>(
            states[i],
            builder,
            storage,
            arguments,
            urd::resolve(registers[i], *search->task->get_repository()));
    };
    // The inputs of each state of the cycle, built before the measured scope.
    const auto contexts = std::array { context_for(0), context_for(1), context_for(2) };
    const auto& context = contexts.front();
    const auto initialize = [&]
    {
        for (auto& evaluator : concepts)
            evaluator.initialize(context);
        for (auto& evaluator : roles)
            evaluator.initialize(context);
        for (auto& evaluator : numericals)
            evaluator.initialize(context);
        for (auto& evaluator : booleans)
            evaluator.initialize(context);
        auto state = evaluation.enter(context);
        for (const auto& root : roots)
            std::visit([&](auto expression) { (void) state.evaluate(graph.get_index(expression)); }, root);
    };
    initialize();
    auto workspace = tyr::datalog::database::Workspace {};
    // Enters state `target_index` by `delta`. Every read root is updated from the delta: get_delta(), delta() and changed()
    // throw otherwise.
    const auto apply = [&](const auto& delta, size_t target_index, size_t members, size_t register_members)
    {
        const auto& target_context = contexts[target_index];
        const auto concept_sizes = std::array { members, members, register_members, static_cast<size_t>(2) };
        const auto role_sizes = std::array { members, members, register_members };
        bool valid = true;
        for (size_t i = 0; i < concepts.size(); ++i)
        {
            auto state = concepts[i].update(target_context, delta, workspace);
            (void) state.get_delta();
            valid &= state.size() == concept_sizes[i];
        }
        for (size_t i = 0; i < roles.size(); ++i)
        {
            auto state = roles[i].update(target_context, delta, workspace);
            (void) state.get_delta();
            valid &= state.size() == role_sizes[i];
        }
        for (size_t i = 0; i < numericals.size(); ++i)
        {
            const auto expected = i < 2 ? members : register_members;
            auto numerical = numericals[i].update(target_context, delta, workspace);
            auto boolean = booleans[i].update(target_context, delta, workspace);
            (void) numerical.changed();
            (void) boolean.changed();
            valid &= numerical.get_result().value() == expected;
            valid &= boolean.get_result().value() == (expected != 0);
        }
        auto graph_state = evaluation.enter(target_context, delta, workspace);
        const auto infinity = std::numeric_limits<ygg::uint_t>::max();
        const auto expected_distance = members == 2 ? static_cast<ygg::uint_t>(2) : members == 1 ? static_cast<ygg::uint_t>(0) : infinity;
        valid &= graph_state.size(source_id) == register_members && graph_state.size(edge_id) == members;
        valid &= graph_state.evaluate(source_count_id).value() == register_members;
        valid &= graph_state.evaluate(edge_count_id).value() == members;
        valid &= graph_state.evaluate(distance_id).value() == expected_distance;
        valid &= graph_state.evaluate(self_distance_id).value() == (register_members ? 0 : infinity);
        valid &= graph_state.size(closure_id) == (members == 2 ? 3 : members);
        valid &= graph_state.evaluate(nonempty_id).value() == (members == 2);
        valid &= graph_state.size(projected_id) == members && graph_state.size(projected_role_id) == members;
        valid &= graph_state.evaluate(query_count_id).value() == members;
        for (const auto id : { source_count_id, edge_count_id, distance_id, self_distance_id, query_count_id })
            (void) graph_state.changed(id);
        (void) graph_state.changed(nonempty_id);
        for (const auto id : { source_id, projected_id })
            (void) graph_state.delta(id);
        for (const auto id : { edge_id, closure_id, projected_role_id })
            (void) graph_state.delta(id);
        return valid;
    };
    const auto cycle = [&]
    {
        bool valid = true;
        for (size_t i = 0; i < changes.size(); ++i)
            valid &= apply(changes[i], i + 1, changes.size() - i - 1, i == 0 ? 1 : 0);
        for (size_t i = changes.size(); i-- > 0;)
        {
            changes[i].reverse();
            valid &= apply(changes[i], i, changes.size() - i, 1);
            changes[i].reverse();
        }
        initialize();
        return valid;
    };

    // Warm both delta directions, first/last memberships, and invocation reinitialization.
    for (size_t repeat = 0; repeat < 8; ++repeat)
        ASSERT_TRUE(cycle());
    bool valid = true;
    const urd::testing::Counted measured;
    for (size_t repeat = 0; repeat < 1000; ++repeat)
        valid &= cycle();
    const auto counts = urd::testing::counts();
    EXPECT_TRUE(valid);
    EXPECT_EQ(counts.allocations, 0);
    EXPECT_EQ(counts.deallocations, 0);
}

TEST(RunirQueries, QueryResultResetReusesCistaSchemaAndRowCapacity)
{
    using ColumnIndex = urd::index<ygg::Column>;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../support/data/kr/dl/query";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto& repository = denotations.get_relation_repository();
    auto dl_builder = kr::dl::semantics::Builder();
    const auto columns = std::array { ygg::ColumnLayout { ColumnIndex(0), ygg::type_id<urd::index<tyr::formalism::Object>> },
                                      ygg::ColumnLayout { ColumnIndex(1), ygg::type_id<urd::index<tyr::formalism::Object>> } };
    const auto renamed = std::array { ygg::ColumnLayout { ColumnIndex(2), ygg::type_id<urd::index<tyr::formalism::Object>> },
                                      ygg::ColumnLayout { ColumnIndex(3), ygg::type_id<urd::index<tyr::formalism::Object>> } };
    auto result = repository.get_relation_builder<ygg::Schema>(columns);
    for (ygg::uint_t i = 0; i < 32; ++i)
        result->insert(std::array<ygg::Value, 2> { ygg::encode_value<ygg::uint_t>(i), ygg::encode_value<ygg::uint_t>(static_cast<ygg::uint_t>(i + 1)) });
    const auto* slot = result.get();
    const auto* schema_buffer = result->columns().data();
    const auto schema_capacity = result->memory_usage() - urd::memory_usage(result->storage());
    const auto* row_buffer = repository.view(*result).row(0).get_values().data();
    const auto row_capacity = urd::memory_usage(result->storage());
    const auto stored = kr::dl::semantics::insert(denotations, *result, dl_builder).first;
    const auto* stored_schema_buffer = stored.columns().data();
    const auto* stored_row_buffer = stored.row(0).get_values().data();
    const auto* stored_row_indices = stored.data().rows.data();
    result = {};

    bool reused = true;
    const urd::testing::Counted measured;
    for (size_t repeat = 0; repeat < 1000; ++repeat)
    {
        denotations.reset();
        reused &= repository.symbols().template size<ygg::Relation<ygg::Schema>>() == 0;
        auto next = repository.get_relation_builder<ygg::Schema>(repeat % 2 ? renamed : columns);
        // Cista schema storage uses malloc, which the global new/delete counter does not cover.
        reused &= next.get() == slot;
        reused &= next->columns().data() == schema_buffer;
        reused &= next->memory_usage() - urd::memory_usage(next->storage()) == schema_capacity;
        reused &= urd::memory_usage(next->storage()) == row_capacity;
        for (ygg::uint_t i = 0; i < 32; ++i)
            next->insert(std::array<ygg::Value, 2> { ygg::encode_value<ygg::uint_t>(i), ygg::encode_value<ygg::uint_t>(static_cast<ygg::uint_t>(i + 1)) });
        reused &= repository.view(*next).row(0).get_values().data() == row_buffer;
        const auto value = kr::dl::semantics::insert(denotations, *next, dl_builder).first;
        reused &= value.columns().data() == stored_schema_buffer;
        reused &= value.row(0).get_values().data() == stored_row_buffer;
        reused &= value.data().rows.data() == stored_row_indices;
    }
    const auto counts = urd::testing::counts();
    EXPECT_TRUE(reused);
    EXPECT_EQ(counts.allocations, 0);
    EXPECT_EQ(counts.deallocations, 0);
}

TEST(RunirQueries, LargeWarmedQueryResultTablesResetWithoutAllocations)
{
    using ColumnIndex = urd::index<ygg::Column>;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../support/data/kr/dl/query";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto& repository = denotations.get_relation_repository();
    auto dl_builder = kr::dl::semantics::Builder();
    const auto columns = std::array { ygg::ColumnLayout { ColumnIndex(0), ygg::type_id<urd::index<tyr::formalism::Object>> },
                                      ygg::ColumnLayout { ColumnIndex(1), ygg::type_id<urd::index<tyr::formalism::Object>> } };
    const auto aliases = std::array { ygg::ColumnLayout { ColumnIndex(100), ygg::type_id<urd::index<tyr::formalism::Object>> },
                                      ygg::ColumnLayout { ColumnIndex(101), ygg::type_id<urd::index<tyr::formalism::Object>> } };
    bool valid = true;
    const auto cycle = [&](size_t generation)
    {
        denotations.reset();
        valid &= repository.symbols().template size<ygg::Relation<ygg::Schema>>() == 0;
        for (ygg::uint_t i = 0; i < 512; ++i)
        {
            const auto object = ObjectIndex(static_cast<ygg::uint_t>(i + generation * 512));
            auto owner = repository.get_relation_builder<ygg::Schema>(columns);
            owner->insert(
                std::array<ygg::Value, 2> { ygg::encode_value<ygg::uint_t>(object.value), ygg::encode_value<ygg::uint_t>(static_cast<ygg::uint_t>(1)) });
            owner->insert(
                std::array<ygg::Value, 2> { ygg::encode_value<ygg::uint_t>(object.value), ygg::encode_value<ygg::uint_t>(static_cast<ygg::uint_t>(2)) });
            const auto value = kr::dl::semantics::insert(denotations, *owner, dl_builder).first;
            const auto alias = kr::dl::semantics::rename(denotations, value, aliases, dl_builder);
            valid &= value.size() == 2 && alias.size() == 2 && ObjectIndex(ygg::decode_value<ygg::uint_t>(value.row(0).value(0))) == object;
            valid &= std::ranges::equal(value.data().rows, alias.data().rows);
            auto duplicate = repository.get_relation_builder<ygg::Schema>(columns);
            duplicate->insert(
                std::array<ygg::Value, 2> { ygg::encode_value<ygg::uint_t>(object.value), ygg::encode_value<ygg::uint_t>(static_cast<ygg::uint_t>(2)) });
            duplicate->insert(
                std::array<ygg::Value, 2> { ygg::encode_value<ygg::uint_t>(object.value), ygg::encode_value<ygg::uint_t>(static_cast<ygg::uint_t>(1)) });
            valid &= kr::dl::semantics::insert(denotations, *duplicate, dl_builder).first == value;
            valid &= kr::dl::semantics::rename(denotations, value, aliases, dl_builder) == alias;
        }
        valid &= repository.symbols().template size<ygg::Relation<ygg::Schema>>() == 1024;
    };
    for (size_t repeat = 0; repeat < 8; ++repeat)
        cycle(repeat);

    const urd::testing::Counted measured;
    for (size_t repeat = 0; repeat < 100; ++repeat)
        cycle(repeat);
    const auto counts = urd::testing::counts();
    EXPECT_TRUE(valid);
    EXPECT_EQ(counts.allocations, 0);
    EXPECT_EQ(counts.deallocations, 0);
}

TEST(RunirSearch, WarmedProgramSearchAllocationsGrowWithContainerCapacity)
{
    using Path = kr::ps::ext::detail::SearchPath<tyr::GroundTag, kr::ps::ext::InternedExecutionStorage<tyr::GroundTag>>;
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../support/data/kr/ps/ext/choose";
    const auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    auto context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(search->task->get_domain()), search);

    for (const auto choose : { false, true })
        for (const auto universal : { false, true })
        {
            SCOPED_TRACE(choose);
            SCOPED_TRACE(universal);
            auto allocations = std::array<size_t, 2> {};
            for (size_t scale = 0; scale < allocations.size(); ++scale)
            {
                const auto length = scale == 0 ? static_cast<size_t>(64) : static_cast<size_t>(512);
                auto text = std::string(R"((:program (:entry chain)
                    (:module (:symbol chain) (:arguments) (:registers (:concept selected))
                        (:entry m0) (:memory)");
                for (size_t i = 0; i <= length + 2; ++i)
                    text += " m" + std::to_string(i);
                text += R"()
                        (:features
                            (:concept (:symbol Candidates) (:expression (c_atomic_state "candidate")))
                            (:concept (:symbol Good) (:expression (c_and (c_atomic_state "candidate") (c_not (c_atomic_state "bad")))))
                            (:concept (:symbol Here) (:expression (c_atomic_state "at")))
                            (:concept (:symbol Goal) (:expression (c_atomic_goal "at" true))))
                        (:rules )";
                for (size_t i = 0; i < length + 2; ++i)
                {
                    text += "(:rule (:symbol r" + std::to_string(i) + ") (:expression (:source-memory m" + std::to_string(i) + ") (:target-memory m"
                            + std::to_string(i + 1) + ") ";
                    if (i < length)
                        text += choose ? "(:choose (:conditions) (:concept Candidates) (:register (:concept selected)))" :
                                         "(:load (:conditions) (:concept Goal) (:register (:concept selected)))";
                    else
                        text += std::string(R"((:do (:conditions) (:action "move") (:arguments Here )") + (i == length ? "Good" : "Goal") + ") (:effects))";
                    text += "))";
                }
                text += ")))";
                const auto program = kr::ps::ext::dl::parse_program(text, search->task->get_domain().get_domain(), *context->domain_context->ext_repository);
                auto options = kr::ps::ext::ProgramSearchOptions<tyr::GroundTag> {};
                options.universal = universal;
                auto expander = kr::ps::ext::SuccessorExpander<tyr::GroundTag>(context, program);
                const auto initial_node = search->successor_generator->get_packed_initial_node(*search->state_repository, *search->axiom_evaluator);
                const auto initial = expander.initial_state(initial_node.get_state().unpack());
                auto classifier = kr::ps::NoUnsolvability<tyr::GroundTag> {};
                auto path_pool = urd::Store<urd::slots<Path>> {};
                // Warm path and binding pools to the live DFS depth. Fresh search storage
                // on each pass ensures memoization cannot skip any of the measured work.
                for (const auto warmup : { true, false })
                {
                    SCOPED_TRACE(warmup);
                    auto storage = kr::ps::ext::detail::SearchStorage<tyr::GroundTag, kr::ps::ext::StateMemorization::ALL>(options);
                    auto statistics = kr::ps::ext::ProgramSearchStatistics {};
                    auto first_goal = std::shared_ptr<Path> {};
                    auto witness = std::shared_ptr<Path> {};
                    const urd::testing::Counted measured;
                    const auto status = kr::ps::ext::detail::depth_first_search<
                        tyr::GroundTag>(expander, initial_node.get_state(), initial, options, classifier, storage, statistics, path_pool, first_goal, witness);
                    const auto counts = urd::testing::counts();
                    if (!warmup)
                        allocations[scale] = static_cast<size_t>(counts.allocations);
                    ASSERT_EQ(status, kr::ps::ProofStatus::SUCCESS);
                    EXPECT_EQ(statistics.num_expanded, length + 2);
                    EXPECT_EQ(statistics.num_generated, length + 2);
                    ASSERT_TRUE(first_goal);
                    EXPECT_EQ(first_goal->choice_depth, choose ? length : 0);
                }
            }
            // After warmup, eight times as many steps must not cause per-step or per-Choose allocations.
            // The allowance covers geometric growth of traversal and memoization containers across allocators.
            EXPECT_LE(allocations[1], allocations[0] + 128) << "small=" << allocations[0] << ", large=" << allocations[1];
        }
}

namespace
{
template<tyr::TaskKind Kind>
urd::testing::Counts measure_warmed_binding_enumeration(const kr::TaskContextPtr<Kind>& context, bool borrowed)
{
    auto& search = *context->search_context;
    auto& generator = *search.successor_generator;
    const auto node = generator.get_initial_node(*search.state_repository, *search.axiom_evaluator);
    bool valid = true;
    const auto enumerate = [&]
    {
        size_t emitted = 0;
        const auto visit = [&](auto)
        {
            ++emitted;
            return true;
        };
        valid &= borrowed ? generator.for_each_borrowed_applicable_action_binding(node, std::ref(visit)) :
                            generator.for_each_applicable_action_binding(node, std::ref(visit));
        valid &= emitted == 2;
    };
    for (size_t i = 0; i < 8; ++i)
        enumerate();
    EXPECT_TRUE(valid);

    const urd::testing::Counted measured;
    for (size_t i = 0; i < 512; ++i)
        enumerate();
    const auto counts = urd::testing::counts();
    EXPECT_TRUE(valid);
    return counts;
}

template<tyr::TaskKind Kind, kr::ps::ext::ExecutionStorageConcept<Kind> Storage>
void expect_warmed_action_expansions(const kr::TaskContextPtr<Kind>& context, kr::ps::ext::ProgramView program, urd::testing::Counts expected)
{
    auto expander = kr::ps::ext::SuccessorExpander<Kind, Storage>(context, program);
    auto& search = *context->search_context;
    const auto node = search.successor_generator->get_initial_node(*search.state_repository, *search.axiom_evaluator);
    const auto initial = expander.initial_state(node.get_state());
    auto statistics = kr::ps::ext::ProgramSearchStatistics {};
    bool valid = true;
    const auto expand = [&]
    {
        size_t emitted = 0;
        valid &= expander.for_each_successor(
            expander.view(initial),
            statistics,
            [&](const auto& step)
            {
                if constexpr (std::same_as<std::remove_cvref_t<decltype(step)>,
                                           kr::ps::ProgramStep<kr::ExtFamilyTag, Kind, kr::ps::ext::StorageStoredState<Storage>>>)
                {
                    valid &= step.status == kr::ps::ProgramOutcome::APPLIED && step.state_transition.has_value();
                    ++emitted;
                }
                else
                    valid = false;
                return true;
            },
            [] { return false; });
        valid &= emitted == 2;
    };
    for (size_t i = 0; i < 8; ++i)
        expand();
    ASSERT_TRUE(valid);

    const urd::testing::Counted measured;
    for (size_t i = 0; i < 512; ++i)
        expand();
    const auto counts = urd::testing::counts();
    EXPECT_TRUE(valid);
    EXPECT_EQ(counts.allocations, expected.allocations);
    EXPECT_EQ(counts.deallocations, expected.deallocations);
}

template<tyr::TaskKind Kind>
void expect_warmed_action_storage()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../support/data/kr/ps/ext/choose";
    const auto search = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
        else
            return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
    }();
    const auto context = kr::TaskContext<Kind>::create(kr::DomainContext::create(search->task->get_domain()), search);
    // Lifted Datalog cost buckets allocate during existing indexed enumeration. Borrowed
    // traversal and policy expansion must add nothing to that measured baseline.
    const auto baseline = measure_warmed_binding_enumeration(context, false);
    const auto borrowed = measure_warmed_binding_enumeration(context, true);
    EXPECT_EQ(borrowed.allocations, baseline.allocations);
    EXPECT_EQ(borrowed.deallocations, baseline.deallocations);
    // This fixture has one action, so the all-schema baseline also covers scoped Do.
    for (const auto& [rule, enumerates] : { std::pair { R"((:action (:conditions) (:action "move") (:query Moves) (:effects (unchanged N))))", false },
                                            std::pair { R"((:do (:conditions) (:action "move") (:arguments From To) (:effects (unchanged N))))", true },
                                            std::pair { R"((:sketch (:conditions) (:effects (unchanged N))))", true } })
    {
        SCOPED_TRACE(rule);
        const auto source = std::string(R"(
(:program (:entry actions)
  (:module (:symbol actions) (:arguments) (:registers)
    (:entry source) (:memory source target)
    (:features
      (:query (:symbol Moves) (:expression
        (q_join (q_atomic_state "edge" (from to)) (q_atomic_state "at" (from)))))
      (:concept (:symbol From) (:expression (c_atomic_state "at")))
      (:concept (:symbol To) (:expression (c_top)))
      (:numerical (:symbol N) (:expression (n_count (c_atomic_state "at")))))
    (:rules (:rule (:symbol move) (:expression
      (:source-memory source) (:target-memory target)
)") + rule + ")))))";
        const auto program = kr::ps::ext::dl::parse_program(source, search->task->get_domain().get_domain(), *context->domain_context->ext_repository);
        const auto expected = enumerates ? baseline : urd::testing::Counts {};
        expect_warmed_action_expansions<Kind, kr::ps::ext::InternedExecutionStorage<Kind>>(context, program, expected);
        expect_warmed_action_expansions<Kind, kr::ps::ext::TransientExecutionStorage<Kind>>(context, program, expected);
    }
}
}  // namespace

TEST(RunirSearch, WarmedActionExpansionsReuseGroundStorage) { expect_warmed_action_storage<tyr::GroundTag>(); }
TEST(RunirSearch, WarmedActionExpansionsReuseLiftedStorage) { expect_warmed_action_storage<tyr::LiftedTag>(); }

}  // namespace runir::tests
