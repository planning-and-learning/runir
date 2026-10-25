#include "incremental_root_evaluators.hpp"
#include "planning_fixtures.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <filesystem>
#include <gtest/gtest.h>
#include <iterator>
#include <ranges>
#include <runir/kr/dl/detail/repository_instantiations.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/evaluation_policy.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/incremental/evaluation.hpp>
#include <runir/kr/dl/semantics/interning.hpp>
#include <runir/kr/dl/semantics/state_evaluation.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <string>
#include <type_traits>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <vector>

namespace runir::tests
{
namespace
{
using Object = ygg::Index<tyr::formalism::Object>;

using GroundContext = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>;
using GroundStateEvaluation = kr::dl::semantics::incremental::StateEvaluation<tyr::GroundTag, GroundContext>;

template<typename Index>
concept HasGraphDelta = requires(GroundStateEvaluation& state, Index index) { state.delta(index); };

template<typename Graph>
concept HasPublicPreparation = requires(Graph& graph, kr::dl::ConstructorView<kr::dl::ConceptTag> expression) { graph.prepare(expression); };

static_assert(!std::default_initializable<kr::dl::semantics::incremental::EvaluationGraph<tyr::GroundTag>>);
static_assert(!std::copy_constructible<kr::dl::semantics::incremental::EvaluationGraph<tyr::GroundTag>>);
static_assert(std::move_constructible<kr::dl::semantics::incremental::EvaluationGraph<tyr::GroundTag>>);
static_assert(!HasPublicPreparation<kr::dl::semantics::incremental::EvaluationGraph<tyr::GroundTag>>);
static_assert(!std::constructible_from<RootEvaluator<tyr::GroundTag, kr::dl::ConceptTag>, kr::dl::ConstructorView<kr::dl::ConceptTag>>);

static_assert(!std::constructible_from<kr::dl::semantics::incremental::EvaluationIndex<kr::dl::RoleTag>,
                                       kr::dl::semantics::incremental::EvaluationIndex<kr::dl::ConceptTag>>);
static_assert(
    !std::constructible_from<kr::dl::semantics::incremental::QueryEvaluationIndex, kr::dl::semantics::incremental::EvaluationIndex<kr::dl::ConceptTag>>);
static_assert(HasGraphDelta<kr::dl::semantics::incremental::EvaluationIndex<kr::dl::ConceptTag>>);
static_assert(HasGraphDelta<kr::dl::semantics::incremental::EvaluationIndex<kr::dl::RoleTag>>);
static_assert(HasGraphDelta<kr::dl::semantics::incremental::QueryEvaluationIndex>);
static_assert(!HasGraphDelta<kr::dl::semantics::incremental::EvaluationIndex<kr::dl::NumericalTag>>);
static_assert(!HasGraphDelta<kr::dl::semantics::incremental::EvaluationIndex<kr::dl::BooleanTag>>);
static_assert(requires(GroundStateEvaluation& state,
                       kr::dl::semantics::incremental::EvaluationIndex<kr::dl::ConceptTag> concept_index,
                       kr::dl::semantics::incremental::EvaluationIndex<kr::dl::RoleTag> role_index) {
    { state.evaluate(concept_index) } -> std::same_as<kr::dl::semantics::BorrowedDenotationView<kr::dl::ConceptTag>>;
    { state.evaluate(role_index) } -> std::same_as<kr::dl::semantics::BorrowedDenotationView<kr::dl::RoleTag>>;
});

template<kr::dl::ConceptOrRoleTag Category>
auto elements(const auto& denotation)
{
    auto result = kr::dl::semantics::DenotationElementViewList<Category> {};
    for (const auto value : denotation)
        result.push_back(value);
    std::ranges::sort(result);
    return result;
}

auto difference(const auto& lhs, const auto& rhs)
{
    auto result = std::remove_cvref_t<decltype(lhs)> {};
    std::ranges::set_difference(lhs, rhs, std::back_inserter(result));
    return result;
}

template<kr::dl::ConceptOrRoleTag Category, tyr::formalism::FactKind Fact>
auto negative_atomic(kr::dl::ConstructorView<Category> positive, kr::dl::ConstructorRepository& repository)
{
    if constexpr (std::same_as<Category, kr::dl::ConceptTag>)
    {
        auto data = positive.get_variant().template get<ygg::Index<kr::dl::Concept<kr::dl::AtomicStateTag<Fact>>>>().get_data();
        data.polarity = false;
        auto wrapper = ygg::Data<kr::dl::Constructor<Category>>(kr::dl::insert(repository, data).first.get_index());
        return kr::dl::insert(repository, wrapper).first;
    }
    else
    {
        auto data = positive.get_variant().template get<ygg::Index<kr::dl::Role<kr::dl::AtomicStateTag<Fact>>>>().get_data();
        data.polarity = false;
        auto wrapper = ygg::Data<kr::dl::Constructor<Category>>(kr::dl::insert(repository, data).first.get_index());
        return kr::dl::insert(repository, wrapper).first;
    }
}

template<tyr::TaskKind Kind>
void check_planning_graph(tyr::planning::StateView<Kind> source, tyr::planning::StateView<Kind> target)
{
    const auto& task = source.get_task();
    auto constructors = kr::dl::ConstructorRepositoryFactory().create(task.get_repository());
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(task.get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto builder = kr::dl::semantics::Builder {};
    auto delta = kr::dl::semantics::incremental::Delta {};
    delta.template assign<Kind>(source, target);
    auto column = ygg::Data<kr::dl::QueryColumn> {};
    column.name = "x";
    const auto x = kr::dl::insert(*constructors, column).first.get_index();
    const auto check = [&]<tyr::formalism::FactKind Fact>()
    {
        for (const auto predicate : task.get_domain().get_domain().template get_predicates<Fact>())
        {
            if (predicate.get_arity() != 1)
                continue;
            auto atom = ygg::Data<kr::dl::Concept<kr::dl::AtomicStateTag<Fact>>>(predicate.get_index(), true);
            auto concept_data = ygg::Data<kr::dl::Constructor<kr::dl::ConceptTag>>(kr::dl::insert(*constructors, atom).first.get_index());
            const auto concept_ = kr::dl::insert(*constructors, concept_data).first;
            auto query_data = ygg::Data<kr::dl::Query<kr::dl::QueryConceptTag>> {};
            query_data.arg = concept_.get_index();
            query_data.columns.push_back(x);
            auto query_wrapper = ygg::Data<kr::dl::Query<>>(kr::dl::insert(*constructors, query_data).first.get_index());
            const auto query = kr::dl::insert(*constructors, query_wrapper).first;
            auto count_data = ygg::Data<kr::dl::Numerical<kr::dl::CountTag>>(query.get_index());
            auto count_wrapper = ygg::Data<kr::dl::Constructor<kr::dl::NumericalTag>>(kr::dl::insert(*constructors, count_data).first.get_index());
            const auto count = kr::dl::insert(*constructors, count_wrapper).first;
            auto evaluation = kr::dl::semantics::incremental::IncrementalEvaluation<Kind>(task, { count });
            const auto concept_id = evaluation.graph().get_index(concept_);
            const auto count_id = evaluation.graph().get_index(count);
            const auto compare = [&](auto& evaluated, auto state)
            {
                storage.reset_dynamic();
                auto context = kr::dl::semantics::StateEvaluationContext<Kind>(state, builder, storage);
                EXPECT_EQ(elements<kr::dl::ConceptTag>(evaluated.evaluate(concept_id)),
                          elements<kr::dl::ConceptTag>(kr::dl::semantics::evaluate<Kind>(concept_, context)));
                EXPECT_EQ(evaluated.evaluate(count_id).get(), kr::dl::semantics::evaluate<Kind>(count, context).get());
            };
            storage.reset_all();
            auto context = kr::dl::semantics::StateEvaluationContext<Kind>(source, builder, storage);
            auto target_context = kr::dl::semantics::StateEvaluationContext<Kind>(target, builder, storage);
            auto& workspace = builder.get_workspace().get_database_workspace();
            auto initial = evaluation.enter(context);
            initial.evaluate(count_id);
            compare(initial, source);
            auto forward = evaluation.enter(target_context, delta, workspace);
            forward.evaluate(count_id);
            EXPECT_NO_THROW(forward.changed(count_id));  // Updated from the delta, so are all its children.
            compare(forward, target);
            auto reversed = delta;
            reversed.reverse();
            auto backward = evaluation.enter(context, reversed, workspace);
            backward.evaluate(count_id);
            EXPECT_NO_THROW(backward.changed(count_id));
            compare(backward, source);
            const auto empty = kr::dl::semantics::incremental::Delta {};
            EXPECT_FALSE(evaluation.enter(context, empty, workspace).changed(count_id));
        }
    };
    check.template operator()<tyr::formalism::FluentTag>();
    check.template operator()<tyr::formalism::DerivedTag>();
}

template<tyr::TaskKind Kind>
void check_incremental_evaluation()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/dl/incremental";
    const auto search = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
        else
            return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
    }();
    auto nodes = std::vector<tyr::planning::Node<Kind>> { search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator) };
    for (size_t i = 0; i < 2; ++i)
    {
        const auto successors = search->successor_generator->get_labeled_successor_nodes(nodes.back(), *search->state_repository, *search->axiom_evaluator);
        ASSERT_FALSE(successors.empty());
        nodes.push_back(successors.front().node);
    }
    check_planning_graph<Kind>(nodes.front().get_state(), nodes.back().get_state());
    const auto domain = search->task->get_domain().get_domain();
    const auto a = domain.get_constants()[0].get_index();
    const auto b = domain.get_constants()[1].get_index();
    const auto c = domain.get_constants()[2].get_index();
    auto constructors = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto builder = kr::dl::semantics::Builder {};
    auto workspace = ygg::database::Workspace {};
    auto register_data = std::array<ygg::Data<kr::dl::semantics::RegisterValues>, 3> {};
    for (auto& data : register_data)
    {
        data.concept_values.resize(2);
        data.role_values.resize(1);
    }
    register_data[0].concept_values[0] = a;
    register_data[0].concept_values[1] = c;
    register_data[0].role_values[0] = cista::pair(a, b);
    register_data[1].concept_values[0] = c;
    register_data[1].concept_values[1] = a;
    register_data[1].role_values[0] = cista::pair(c, a);
    const auto registers = [&](size_t i) { return ygg::make_view(register_data[i], *search->task->get_repository()); };

    // Invocation values live outside the repositories reset during ordinary evaluation.
    auto argument_concept = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(3);
    auto argument_role = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(3);
    auto argument_boolean = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>(true);
    auto argument_numerical = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>(7);
    argument_concept.get().set(ygg::uint_t(a));
    argument_role.get(a).set(ygg::uint_t(b));
    auto argument_data = ygg::Data<kr::dl::semantics::CallArguments> {};
    argument_data.concept_arguments.push_back(kr::dl::semantics::insert(denotations, argument_concept, builder).first.get_index());
    argument_data.role_arguments.push_back(kr::dl::semantics::insert(denotations, argument_role, builder).first.get_index());
    argument_data.boolean_arguments.push_back(kr::dl::semantics::insert(denotations, argument_boolean, builder).first.get_index());
    argument_data.numerical_arguments.push_back(kr::dl::semantics::insert(denotations, argument_numerical, builder).first.get_index());
    const auto arguments = kr::dl::semantics::insert(denotations, argument_data).first;
    argument_concept.get().reset(ygg::uint_t(a));
    argument_concept.get().set(ygg::uint_t(c));
    argument_role.get(a).reset(ygg::uint_t(b));
    argument_role.get(c).set(ygg::uint_t(a));
    argument_boolean.value = false;
    argument_numerical.value = 2;
    argument_data.concept_arguments[0] = kr::dl::semantics::insert(denotations, argument_concept, builder).first.get_index();
    argument_data.role_arguments[0] = kr::dl::semantics::insert(denotations, argument_role, builder).first.get_index();
    argument_data.boolean_arguments[0] = kr::dl::semantics::insert(denotations, argument_boolean, builder).first.get_index();
    argument_data.numerical_arguments[0] = kr::dl::semantics::insert(denotations, argument_numerical, builder).first.get_index();
    const auto other_arguments = kr::dl::semantics::insert(denotations, argument_data).first;
    const auto context_for = [&](auto state, size_t i, kr::dl::semantics::CallArgumentsView invocation)
    { return kr::dl::semantics::StateEvaluationContext<Kind, decltype(state)>(state, builder, storage, invocation, registers(i)); };
    auto deltas = std::array<kr::dl::semantics::incremental::Delta, 2> {};
    for (size_t i = 0; i < deltas.size(); ++i)
        deltas[i].template assign<Kind>(nodes[i].get_state(), registers(i), nodes[i + 1].get_state(), registers(i + 1));
    // Count regressions need a replacement with unchanged cardinality, followed by removal to empty.
    ASSERT_EQ(deltas[0].added.concept_registers.size(), 2);
    ASSERT_EQ(deltas[0].removed.concept_registers.size(), 2);
    ASSERT_EQ(deltas[0].added.role_registers.size(), 1);
    ASSERT_EQ(deltas[0].removed.role_registers.size(), 1);
    ASSERT_TRUE(deltas[1].added.concept_registers.empty());
    ASSERT_EQ(deltas[1].removed.concept_registers.size(), 2);
    ASSERT_TRUE(deltas[1].added.role_registers.empty());
    ASSERT_EQ(deltas[1].removed.role_registers.size(), 1);
    const auto empty_delta = kr::dl::semantics::incremental::Delta {};
    const auto expect_unpublished = [&]
    {
        const auto& repository = storage.get_denotation_repository(false);
        EXPECT_EQ(repository.template size<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(), 0);
        EXPECT_EQ(repository.template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
        EXPECT_EQ(repository.template size<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>(), 0);
        EXPECT_EQ(repository.template size<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>(), 0);
        EXPECT_TRUE(repository.get_relation_repository().empty());
        EXPECT_TRUE(storage.get_caches().template get<kr::dl::ConceptTag>(false).empty());
        EXPECT_TRUE(storage.get_caches().template get<kr::dl::RoleTag>(false).empty());
        EXPECT_TRUE(storage.get_caches().template get<kr::dl::BooleanTag>(false).empty());
        EXPECT_TRUE(storage.get_caches().template get<kr::dl::NumericalTag>(false).empty());
    };

    const auto check = [&]<kr::dl::CategoryTag Category>(kr::dl::ConstructorView<Category> expression)
    {
        auto evaluator = RootEvaluator<Kind, Category>(*search->task, expression);
        const auto snapshot = [&](const auto& result)
        {
            if constexpr (kr::dl::ConceptOrRoleTag<Category>)
                return elements<Category>(result);
            else
                return result.get();
        };
        const auto expect_empty_delta = [&](auto& state)
        {
            if constexpr (kr::dl::ConceptOrRoleTag<Category>)
            {
                EXPECT_TRUE(state.get_delta().added.empty());
                EXPECT_TRUE(state.get_delta().removed.empty());
            }
            else
                EXPECT_FALSE(state.changed());
        };
        // An initialized result has no delta; only an update from a delta has one.
        const auto expect_no_delta = [&](auto& state)
        {
            if constexpr (kr::dl::ConceptOrRoleTag<Category>)
                EXPECT_THROW(state.get_delta(), std::logic_error);
            else
                EXPECT_THROW(state.changed(), std::logic_error);
        };
        const auto compare_full = [&](auto& state, auto& context)
        {
            const auto expected = kr::dl::semantics::evaluate<Kind>(expression, context);
            if constexpr (kr::dl::ConceptOrRoleTag<Category>)
            {
                EXPECT_EQ(snapshot(state.get_result()), elements<Category>(expected));
                EXPECT_EQ(state.size(), snapshot(state.get_result()).size());
                if constexpr (std::same_as<Category, kr::dl::ConceptTag>)
                    EXPECT_EQ(state.get_result().get().count(), state.size());
                else
                    EXPECT_EQ(state.get_result().count(), state.size());
            }
            else
                EXPECT_EQ(snapshot(state.get_result()), expected.get());
        };
        storage.reset_all();
        auto initial_context = context_for(nodes.front().get_state(), 0, arguments);
        // A fresh evaluator has no previous state: an update initializes the root, so it has no delta.
        auto fresh = evaluator.update(initial_context, empty_delta, workspace);
        expect_no_delta(fresh);
        auto initial = evaluator.initialize(initial_context);
        const auto retained_view = initial.get_result();
        const ygg::uint_t* blocks = nullptr;
        if constexpr (kr::dl::ConceptOrRoleTag<Category>)
            blocks = retained_view.get_handle().blocks.data();
        expect_no_delta(initial);
        expect_unpublished();
        compare_full(initial, initial_context);
        // Updates from the previous state, whose result is `before`, and returns the result of the reached state.
        const auto update = [&](const auto& delta, size_t i, const auto& before)
        {
            storage.reset_dynamic();
            auto context = context_for(nodes[i].get_state(), i, arguments);
            auto state = evaluator.update(context, delta, workspace);
            expect_unpublished();
            // get_delta() and changed() throw unless the root was updated from the delta.
            if constexpr (kr::dl::ConceptOrRoleTag<Category>)
            {
                EXPECT_EQ(state.get_result().get_handle().blocks.data(), blocks);
                EXPECT_EQ(elements<Category>(retained_view), snapshot(state.get_result()));
                EXPECT_TRUE(std::ranges::is_permutation(state.get_delta().added, difference(snapshot(state.get_result()), before)));
                EXPECT_TRUE(std::ranges::is_permutation(state.get_delta().removed, difference(before, snapshot(state.get_result()))));
            }
            else
            {
                EXPECT_EQ(retained_view.get(), snapshot(state.get_result()));
                EXPECT_EQ(state.changed(), before != snapshot(state.get_result()));
            }
            compare_full(state, context);
            auto after = snapshot(state.get_result());
            auto unchanged = evaluator.update(context, empty_delta, workspace);
            expect_empty_delta(unchanged);
            EXPECT_EQ(snapshot(unchanged.get_result()), after);
            return after;
        };
        auto current = snapshot(initial.get_result());
        for (size_t i = 0; i < deltas.size(); ++i)
            current = update(deltas[i], i + 1, current);
        for (size_t i = deltas.size(); i > 0; --i)
        {
            auto reversed = deltas[i - 1];
            reversed.reverse();
            current = update(reversed, i - 1, current);
        }

        // Resetting repositories cannot invalidate borrowed evaluator results. Reinitializing
        // on the same task retains static results but reloads invocation arguments.
        const auto before_reset = snapshot(retained_view);
        EXPECT_EQ(before_reset, current);
        storage.reset_all();
        EXPECT_EQ(snapshot(retained_view), before_reset);
        const auto borrowed_builder = nodes.back().get_state().get_state_builder();
        auto final_context = context_for(ygg::make_view(borrowed_builder, *search->task), 2, other_arguments);
        auto final_state = evaluator.initialize(final_context);
        expect_no_delta(final_state);
        expect_unpublished();
        EXPECT_TRUE(storage.get_denotation_repository(true).get_vector_repository().empty());
        EXPECT_TRUE(storage.get_caches().template get<Category>(true).empty());
        compare_full(final_state, final_context);

        // Demands may skip states, move backwards, and reenter with new arguments.
        const auto roots = std::array<kr::dl::semantics::incremental::EvaluationRoot, 1> { expression };
        auto policy = kr::dl::semantics::DeltaEvaluationPolicy<Kind>(*search->task, roots);
        const auto compare_demand = [&](auto& context)
        {
            auto demand = policy.make_source_context(context);
            const auto actual = kr::dl::semantics::evaluate<Kind>(expression, demand);
            const auto expected = kr::dl::semantics::evaluate<Kind>(expression, context);
            if constexpr (kr::dl::ConceptOrRoleTag<Category>)
                EXPECT_EQ(elements<Category>(actual), elements<Category>(expected));
            else
                EXPECT_EQ(actual.get(), expected.get());
            const auto repeated = kr::dl::semantics::evaluate<Kind>(expression, demand);
            if constexpr (kr::dl::ConceptOrRoleTag<Category>)
                EXPECT_EQ(elements<Category>(actual), elements<Category>(repeated));
            else
                EXPECT_EQ(actual.get(), repeated.get());
        };
        const auto states = std::array<size_t, 7> { 0, 1, 2, 1, 0, 2, 0 };
        for (size_t epoch = 0; epoch < states.size(); ++epoch)
        {
            storage.reset_all();
            policy.reset_source();
            const auto i = states[epoch];
            auto context = context_for(nodes[i].get_state(), i, arguments);
            if (epoch == 2 || epoch == 5)
                (void) policy.make_source_context(context);
            else
                compare_demand(context);
        }
        policy.invalidate();
        storage.reset_all();
        compare_demand(final_context);
    };

    const auto check_leaf = [&]<kr::dl::ConceptOrRoleTag Category>(kr::dl::ConstructorView<Category> expression)
    {
        check(expression);
        auto count = ygg::Data<kr::dl::Numerical<kr::dl::CountTag>>(expression.get_index());
        auto count_wrapper = ygg::Data<kr::dl::Constructor<kr::dl::NumericalTag>>(kr::dl::insert(*constructors, count).first.get_index());
        check(kr::dl::insert(*constructors, count_wrapper).first);
        auto nonempty = ygg::Data<kr::dl::Boolean<kr::dl::NonemptyTag>>(expression.get_index());
        auto nonempty_wrapper = ygg::Data<kr::dl::Constructor<kr::dl::BooleanTag>>(kr::dl::insert(*constructors, nonempty).first.get_index());
        check(kr::dl::insert(*constructors, nonempty_wrapper).first);
    };
    const auto present = kr::ps::ext::dl::parse_concept(R"((c_atomic_state "present"))", domain, *constructors);
    const auto copied_present = kr::ps::ext::dl::parse_concept(R"((c_atomic_state "copied-present"))", domain, *constructors);
    const auto edge = kr::ps::ext::dl::parse_role(R"((r_atomic_state "edge"))", domain, *constructors);
    const auto copied_edge = kr::ps::ext::dl::parse_role(R"((r_atomic_state "copied-edge"))", domain, *constructors);
    const auto concept_expressions = std::vector {
        present,
        copied_present,
        negative_atomic<kr::dl::ConceptTag, tyr::formalism::FluentTag>(present, *constructors),
        negative_atomic<kr::dl::ConceptTag, tyr::formalism::DerivedTag>(copied_present, *constructors),
        kr::ps::ext::dl::parse_concept("(c_register 0)", domain, *constructors),
        kr::ps::ext::dl::parse_concept("(c_argument 0)", domain, *constructors),
        kr::ps::ext::dl::parse_concept(R"((c_and (c_atomic_state "fixed") (c_not (c_nominal "a"))))", domain, *constructors),
        kr::ps::ext::dl::parse_concept(R"((c_atomic_goal "present" true))", domain, *constructors),
        kr::ps::ext::dl::parse_concept(R"((c_atomic_goal "present" false))", domain, *constructors),
    };
    for (size_t i = 0; i < concept_expressions.size(); ++i)
    {
        SCOPED_TRACE("concept expression " + std::to_string(i));
        check_leaf(concept_expressions[i]);
    }
    const auto present_text = std::string(R"((c_atomic_state "present"))");
    const auto edge_text = std::string(R"((r_atomic_state "edge"))");
    const auto additional_concepts = std::vector<std::string> {
        "(c_bot)",
        "(c_top)",
        "(c_and " + present_text + " " + present_text + ")",
        "(c_or " + present_text + " (c_register 0))",
        "(c_not " + present_text + ")",
        "(c_all " + edge_text + " " + present_text + ")",
        "(c_some " + edge_text + " " + present_text + ")",
        "(c_subset " + edge_text + " (r_register 0))",
        "(c_same_as " + edge_text + " (r_register 0))",
        "(c_fillers " + edge_text + " \"b\")",
        R"((c_one_of "a" "c"))",
        R"((c_project y (q_atomic_state "edge" (x y))))",
        "(c_project x (q_join (q_role (x y) " + edge_text + ") (q_concept z " + present_text + ")))",
    };
    for (const auto& text : additional_concepts)
    {
        SCOPED_TRACE(text);
        check_leaf(kr::ps::ext::dl::parse_concept(text, domain, *constructors));
    }
    for (const auto* operation : { "c_at_least", "c_at_most", "c_exactly" })
        for (const auto threshold : { 0, 1, 2 })
        {
            const auto prefix = "(" + std::string(operation) + " " + std::to_string(threshold) + " " + edge_text;
            SCOPED_TRACE(prefix);
            check_leaf(kr::ps::ext::dl::parse_concept(prefix + ")", domain, *constructors));
            check_leaf(kr::ps::ext::dl::parse_concept(std::string(prefix).append(" ").append(present_text).append(")"), domain, *constructors));
        }
    const auto role_expressions = std::vector {
        edge,
        copied_edge,
        negative_atomic<kr::dl::RoleTag, tyr::formalism::FluentTag>(edge, *constructors),
        negative_atomic<kr::dl::RoleTag, tyr::formalism::DerivedTag>(copied_edge, *constructors),
        kr::ps::ext::dl::parse_role("(r_register 0)", domain, *constructors),
        kr::ps::ext::dl::parse_role("(r_argument 0)", domain, *constructors),
        kr::ps::ext::dl::parse_role("(r_inverse (r_universal))", domain, *constructors),
        kr::ps::ext::dl::parse_role(R"((r_atomic_goal "edge" true))", domain, *constructors),
        kr::ps::ext::dl::parse_role(R"((r_atomic_goal "edge" false))", domain, *constructors),
    };
    for (size_t i = 0; i < role_expressions.size(); ++i)
    {
        SCOPED_TRACE("role expression " + std::to_string(i));
        check_leaf(role_expressions[i]);
    }
    const auto additional_roles = std::vector<std::string> {
        "(r_and " + edge_text + " " + edge_text + ")",
        "(r_or " + edge_text + " (r_register 0))",
        "(r_complement " + edge_text + ")",
        "(r_inverse " + edge_text + ")",
        "(r_composition " + edge_text + " " + edge_text + ")",
        "(r_transitive_closure " + edge_text + ")",
        "(r_reflexive_transitive_closure " + edge_text + ")",
        "(r_restriction " + edge_text + " " + present_text + ")",
        "(r_identity " + present_text + ")",
        R"((r_project y x (q_atomic_state "edge" (x y))))",
        "(r_project x y (q_join (q_role (x y) " + edge_text + ") (q_concept z " + present_text + ")))",
    };
    for (const auto& text : additional_roles)
    {
        SCOPED_TRACE(text);
        check_leaf(kr::ps::ext::dl::parse_role(text, domain, *constructors));
    }
    check(kr::ps::ext::dl::parse_boolean("(b_argument 0)", domain, *constructors));
    check(kr::ps::ext::dl::parse_numerical("(n_argument 0)", domain, *constructors));
    check(kr::ps::ext::dl::parse_numerical(R"((n_count (q_atomic_state "edge" (x y))))", domain, *constructors));
    // Query cardinality follows 1→1→0 and undo. Count and nonempty must report unchanged
    // on replacement/no-op, and reload the empty baseline on reinitialization.
    for (const auto* text : { "(q_concept x (c_register 0))", "(q_role (x y) (r_register 0))" })
    {
        SCOPED_TRACE(text);
        check(kr::ps::ext::dl::parse_numerical("(n_count " + std::string(text) + ")", domain, *constructors));
        check(kr::ps::ext::dl::parse_boolean("(b_nonempty " + std::string(text) + ")", domain, *constructors));
    }
    check(kr::ps::ext::dl::parse_boolean(R"((b_nonempty (q_atomic_state "edge" (x y))))", domain, *constructors));
    for (const auto* predicate : { "ready", "copied-ready" })
        for (const auto* polarity : { "true", "false" })
            check(kr::ps::ext::dl::parse_boolean("(b_atomic_state \"" + std::string(predicate) + "\" " + polarity + ")", domain, *constructors));
    const auto count_present = "(n_count " + present_text + ")";
    for (const auto* operation : { "n_add", "n_sub", "n_mul", "n_div", "n_min", "n_max" })
    {
        const auto text = "(" + std::string(operation) + " " + count_present + " (n_argument 0))";
        SCOPED_TRACE(text);
        check(kr::ps::ext::dl::parse_numerical(text, domain, *constructors));
    }
    const auto distance = "(n_distance (c_register 0) " + edge_text + " (c_register 1))";
    const auto additional_numericals = std::vector<std::string> {
        "(n_const 0)",
        "(n_distance " + present_text + " " + edge_text + " (c_nominal \"c\"))",
        "(n_distance " + present_text + " " + edge_text + " " + present_text + ")",
        // Sources and targets may use closures; only the edges are search-free.
        "(n_distance (c_some (r_transitive_closure " + edge_text + ") " + present_text + ") " + edge_text + " (c_register 1))",
        "(n_distance (c_register 0) " + edge_text + " (c_some (r_reflexive_transitive_closure " + edge_text + ") (c_register 1)))",
        distance,
        "(n_add " + distance + " (n_const 1))",
        "(n_div (n_count " + edge_text + ") " + count_present + ")",
        // Query arguments (k = 1), mixed forms, k = 2 tuple vertices, unreachable and equal endpoints.
        "(n_distance (q_concept x (c_register 0)) (q_role (x y) " + edge_text + ") (q_concept x (c_register 1)))",
        "(n_distance (c_register 0) (q_role (x y) " + edge_text + ") (c_register 1))",
        "(n_distance (q_concept x " + present_text + ") " + edge_text + " (c_register 1))",
        "(n_distance (q_role (x u) (r_identity (c_register 0))) (q_project (x u y v) (q_join (q_role (x y) " + edge_text + ") (q_role (u v) " + edge_text
            + "))) (q_role (x u) (r_identity (c_register 1))))",
        // k = 2 edges filtered by a second dynamic leaf: the evaluator must notice either change.
        "(n_distance (q_role (x u) (r_identity (c_register 0))) (q_project (x u y v) (q_join (q_join (q_role (x y) " + edge_text + ") (q_role (u v) "
            + edge_text + ")) (q_concept y " + present_text + "))) (q_role (x u) (r_identity (c_register 1))))",
        "(n_distance (q_concept x (c_register 0)) (q_role (x y) (r_identity (c_bot))) (q_concept x (c_register 1)))",
        "(n_distance (q_concept x (c_register 0)) (q_role (x y) " + edge_text + ") (q_concept x (c_register 0)))",
    };
    for (const auto& text : additional_numericals)
    {
        SCOPED_TRACE(text);
        check(kr::ps::ext::dl::parse_numerical(text, domain, *constructors));
    }

    // Module features share their operands across counts, distances, and query conversions.
    const auto source = kr::ps::ext::dl::parse_concept("(c_register 0)", domain, *constructors);
    const auto target = kr::ps::ext::dl::parse_concept("(c_register 1)", domain, *constructors);
    const auto source_count = kr::ps::ext::dl::parse_numerical("(n_count (c_register 0))", domain, *constructors);
    const auto edge_count = kr::ps::ext::dl::parse_numerical("(n_count " + edge_text + ")", domain, *constructors);
    const auto distance_expression = kr::ps::ext::dl::parse_numerical(distance, domain, *constructors);
    const auto self_distance = kr::ps::ext::dl::parse_numerical("(n_distance (c_register 0) " + edge_text + " (c_register 0))", domain, *constructors);
    const auto query_count = kr::ps::ext::dl::parse_numerical("(n_count (q_role (x y) " + edge_text + "))", domain, *constructors);
    const auto query =
        query_count.get_variant().template get<ygg::Index<kr::dl::Numerical<kr::dl::CountTag>>>().get_arg().template get<ygg::Index<kr::dl::Query<>>>();
    // Duplicate roots and explicitly requested shared children do not duplicate nodes.
    auto evaluation = kr::dl::semantics::incremental::IncrementalEvaluation<Kind>(
        *search->task,
        { source_count, edge_count, distance_expression, self_distance, query_count, query, distance_expression, source, edge });
    const auto& graph = evaluation.graph();
    const auto source_id = graph.get_index(source);
    const auto edge_id = graph.get_index(edge);
    const auto target_id = graph.get_index(target);  // A prepared child can be inspected without making it another root.
    const auto source_count_id = graph.get_index(source_count);
    const auto edge_count_id = graph.get_index(edge_count);
    const auto distance_id = graph.get_index(distance_expression);
    const auto self_distance_id = graph.get_index(self_distance);
    const auto query_id = graph.get_index(query);
    const auto query_count_id = graph.get_index(query_count);
    // Nine expression nodes plus the three relations lifted for the distance arguments; the
    // self distance reuses the same lifts.
    EXPECT_EQ(graph.node_count(), 12);
    EXPECT_EQ(graph.get_index(distance_expression), distance_id);
    EXPECT_EQ(graph.get_index(source), source_id);
    EXPECT_EQ(graph.get_index(edge), edge_id);
    EXPECT_EQ(graph.get_index(query), query_id);
    const auto unprepared = kr::ps::ext::dl::parse_numerical("(n_const 42)", domain, *constructors);
    EXPECT_THROW(graph.get_index(unprepared), std::invalid_argument);
    EXPECT_EQ(graph.node_count(), 12);
    const auto scalar_expressions = std::array { source_count, edge_count, distance_expression, self_distance, query_count };
    const auto scalar_ids = std::array { source_count_id, edge_count_id, distance_id, self_distance_id, query_count_id };
    const auto scalar_values = [&](auto& state)
    {
        auto values = std::array<ygg::uint_t, 5> {};
        for (size_t i = 0; i < values.size(); ++i)
            values[i] = state.evaluate(scalar_ids[i]).get();
        return values;
    };
    const auto query_rows = [](const auto& relation)
    {
        auto result = std::vector<std::vector<Object>> {};
        for (size_t i = 0; i < relation.size(); ++i)
        {
            const auto row = ygg::database::Row(relation.row(i), relation.columns().span());
            auto& values = result.emplace_back();
            for (size_t j = 0; j < row.size(); ++j)
                values.push_back(Object(row.get<ygg::uint_t>(j)));
        }
        std::ranges::sort(result);
        return result;
    };
    const auto demand_all = [&](auto& state)
    {
        (void) state.evaluate(source_id);
        (void) state.evaluate(target_id);
        (void) state.evaluate(edge_id);
        for (const auto id : scalar_ids)
            (void) state.evaluate(id);
        (void) state.evaluate(query_id);
    };
    const auto compare_graph = [&](auto& state, auto& context)
    {
        EXPECT_EQ(elements<kr::dl::ConceptTag>(state.evaluate(source_id)), elements<kr::dl::ConceptTag>(kr::dl::semantics::evaluate<Kind>(source, context)));
        EXPECT_EQ(elements<kr::dl::ConceptTag>(state.evaluate(target_id)), elements<kr::dl::ConceptTag>(kr::dl::semantics::evaluate<Kind>(target, context)));
        EXPECT_EQ(elements<kr::dl::RoleTag>(state.evaluate(edge_id)), elements<kr::dl::RoleTag>(kr::dl::semantics::evaluate<Kind>(edge, context)));
        for (size_t i = 0; i < scalar_ids.size(); ++i)
            EXPECT_EQ(state.evaluate(scalar_ids[i]).get(), kr::dl::semantics::evaluate<Kind>(scalar_expressions[i], context).get());
        EXPECT_EQ(query_rows(state.evaluate(query_id)), query_rows(kr::dl::semantics::evaluate<Kind>(query, context)));
    };
    storage.reset_all();
    auto initial_context = context_for(nodes.front().get_state(), 0, arguments);
    // Copying the task preserves the formalism repository but changes task identity.
    const auto other_task = *search->task;
    const auto borrowed_builder = nodes.front().get_state().get_state_builder();
    auto foreign_context = context_for(ygg::make_view(borrowed_builder, other_task), 0, arguments);
    const auto policy_roots = std::array<kr::dl::semantics::incremental::EvaluationRoot, 1> { source_count };
    auto policy = kr::dl::semantics::DeltaEvaluationPolicy<Kind>(*search->task, policy_roots);
    EXPECT_NO_THROW(policy.make_source_context(initial_context));
    EXPECT_THROW(policy.make_source_context(foreign_context), std::invalid_argument);
    policy.reset_source();
    EXPECT_THROW(policy.make_source_context(foreign_context), std::invalid_argument);
    EXPECT_NO_THROW(policy.make_source_context(initial_context));
    EXPECT_THROW(evaluation.enter(foreign_context), std::invalid_argument);
    auto initial = evaluation.enter(initial_context);
    EXPECT_THROW(evaluation.enter(foreign_context), std::invalid_argument);
    // Rejection must preserve the entered state and its prepared handles.
    EXPECT_EQ(graph.get_index(source), source_id);
    demand_all(initial);
    expect_unpublished();
    compare_graph(initial, initial_context);
    const auto source_view = initial.evaluate(source_id);
    const auto edge_view = initial.evaluate(edge_id);
    const auto* source_blocks = source_view.get_handle().blocks.data();
    const auto* edge_blocks = edge_view.get_handle().blocks.data();
    // The previous state's query rows and scalars, read from its handle before entering the next state.
    auto before_query = query_rows(initial.evaluate(query_id));
    auto before_scalars = scalar_values(initial);
    const auto update_graph = [&](const auto& delta, size_t i)
    {
        const auto before_source = elements<kr::dl::ConceptTag>(source_view);
        const auto before_edge = elements<kr::dl::RoleTag>(edge_view);
        storage.reset_dynamic();
        auto context = context_for(nodes[i].get_state(), i, arguments);
        auto state = evaluation.enter(context, delta, workspace);
        demand_all(state);
        expect_unpublished();
        EXPECT_EQ(source_view.get_handle().blocks.data(), source_blocks);
        EXPECT_EQ(edge_view.get_handle().blocks.data(), edge_blocks);
        const auto after_source = elements<kr::dl::ConceptTag>(source_view);
        const auto after_edge = elements<kr::dl::RoleTag>(edge_view);
        EXPECT_TRUE(std::ranges::is_permutation(state.delta(source_id).added, difference(after_source, before_source)));
        EXPECT_TRUE(std::ranges::is_permutation(state.delta(source_id).removed, difference(before_source, after_source)));
        EXPECT_TRUE(std::ranges::is_permutation(state.delta(edge_id).added, difference(after_edge, before_edge)));
        EXPECT_TRUE(std::ranges::is_permutation(state.delta(edge_id).removed, difference(before_edge, after_edge)));
        EXPECT_EQ(query_rows(state.delta(query_id).added), difference(query_rows(state.evaluate(query_id)), before_query));
        EXPECT_EQ(query_rows(state.delta(query_id).removed), difference(before_query, query_rows(state.evaluate(query_id))));
        for (size_t j = 0; j < scalar_ids.size(); ++j)
            EXPECT_EQ(state.changed(scalar_ids[j]), before_scalars[j] != scalar_values(state)[j]);
        compare_graph(state, context);
        const auto after_scalars = scalar_values(state);
        auto unchanged = evaluation.enter(context, empty_delta, workspace);
        demand_all(unchanged);
        EXPECT_TRUE(unchanged.delta(source_id).added.empty());
        EXPECT_TRUE(unchanged.delta(source_id).removed.empty());
        EXPECT_TRUE(unchanged.delta(edge_id).added.empty());
        EXPECT_TRUE(unchanged.delta(edge_id).removed.empty());
        EXPECT_TRUE(unchanged.delta(query_id).added.empty());
        EXPECT_TRUE(unchanged.delta(query_id).removed.empty());
        for (const auto id : scalar_ids)
            EXPECT_FALSE(unchanged.changed(id));
        EXPECT_EQ(scalar_values(unchanged), after_scalars);
        before_query = query_rows(unchanged.evaluate(query_id));
        before_scalars = scalar_values(unchanged);
    };
    for (size_t i = 0; i < deltas.size(); ++i)
        update_graph(deltas[i], i + 1);
    for (size_t i = deltas.size(); i > 0; --i)
    {
        auto reversed = deltas[i - 1];
        reversed.reverse();
        update_graph(reversed, i - 1);
    }
    storage.reset_all();
    auto invocation_context = context_for(nodes.back().get_state(), 2, other_arguments);
    auto invocation = evaluation.enter(invocation_context);
    demand_all(invocation);
    expect_unpublished();
    for (const auto id : scalar_ids)
        EXPECT_THROW(invocation.changed(id), std::logic_error);  // Initialized results have no delta.
    compare_graph(invocation, invocation_context);

    // This invocation has empty registers. A failed update leaves the failed node unevaluated; the state's inputs
    // initialize it on its next demand. A node not evaluated in the previous state is initialized rather than
    // updated, so its delta throws in the next one.
    auto invalid_delta = kr::dl::semantics::incremental::Delta {};
    invalid_delta.removed.concept_registers.emplace_back(kr::dl::RegisterIdentifier<kr::dl::ConceptTag>(0), ygg::make_view(a, *search->task->get_repository()));
    auto failed = evaluation.enter(invocation_context, invalid_delta, workspace);
    EXPECT_THROW(failed.evaluate(source_id), std::invalid_argument);
    auto after_failure = evaluation.enter(invocation_context, empty_delta, workspace);
    EXPECT_THROW(after_failure.delta(source_id), std::logic_error);
    EXPECT_THROW(after_failure.delta(query_id), std::logic_error);  // Neither the query nor its child were demanded.
    auto reentered = evaluation.enter(invocation_context);
    compare_graph(reentered, invocation_context);
    // Reading a failed node again in the same state initializes it from the state's inputs. (Checked in a state of its
    // own: in the state above, it would make source_id evaluated, so its delta would no longer throw after_failure.)
    auto failed_again = evaluation.enter(invocation_context, invalid_delta, workspace);
    EXPECT_THROW(failed_again.evaluate(source_id), std::invalid_argument);
    EXPECT_EQ(elements<kr::dl::ConceptTag>(failed_again.evaluate(source_id)),
              elements<kr::dl::ConceptTag>(kr::dl::semantics::evaluate<Kind>(source, invocation_context)));

    // Roots are evaluated on demand only; a later parent starts from its current child. Whether an unrelated root
    // stayed unevaluated in a state is not observable by design: every read demands its node first.
    storage.reset_all();
    auto demanded = evaluation.enter(initial_context);
    EXPECT_EQ(query_rows(demanded.evaluate(query_id)), query_rows(kr::dl::semantics::evaluate<Kind>(query, initial_context)));
    EXPECT_EQ(demanded.evaluate(edge_count_id).get(), kr::dl::semantics::evaluate<Kind>(edge_count, initial_context).get());
    EXPECT_THROW(demanded.delta(query_id), std::logic_error);  // Initialized results have no delta.
    size_t previous_state = 0;
    auto demand_delta = kr::dl::semantics::incremental::Delta {};
    for (const auto i : { static_cast<size_t>(1), static_cast<size_t>(2), static_cast<size_t>(0) })
    {
        storage.reset_dynamic();
        demand_delta.template assign<Kind>(nodes[previous_state].get_state(), registers(previous_state), nodes[i].get_state(), registers(i));
        auto context = context_for(nodes[i].get_state(), i, arguments);
        auto state = evaluation.enter(context, demand_delta, workspace);
        EXPECT_EQ(query_rows(state.evaluate(query_id)), query_rows(kr::dl::semantics::evaluate<Kind>(query, context)));
        EXPECT_NO_THROW(state.delta(query_id));  // Updated from the delta.
        EXPECT_EQ(state.evaluate(edge_count_id).get(), kr::dl::semantics::evaluate<Kind>(edge_count, context).get());
        if (i == 0)
        {
            // Unrelated roots remained unmaterialized in the previous state: the parent of the exact query is
            // initialized, and so is the register the distance depends on.
            EXPECT_THROW(state.changed(query_count_id), std::logic_error);
            EXPECT_THROW(state.delta(source_id), std::logic_error);
            EXPECT_EQ(state.evaluate(query_count_id).get(), kr::dl::semantics::evaluate<Kind>(query_count, context).get());
            const auto retained_rows = query_rows(state.evaluate(query_id));
            for (size_t j = 0; j < scalar_ids.size(); ++j)
                EXPECT_EQ(state.evaluate(scalar_ids[j]).get(), kr::dl::semantics::evaluate<Kind>(scalar_expressions[j], context).get());
            EXPECT_EQ(query_rows(state.evaluate(query_id)), retained_rows);
        }
        previous_state = i;
    }
    EXPECT_THROW(demanded.evaluate(query_id), std::logic_error);  // A handle of an earlier state is stale.
    auto fresh = evaluation.enter(invocation_context);
    storage.reset_all();
    EXPECT_EQ(fresh.evaluate(query_count_id).get(), kr::dl::semantics::evaluate<Kind>(query_count, invocation_context).get());
    const auto reject_multiple_values = [&]<kr::dl::ConceptOrRoleTag Category>(kr::dl::ConstructorView<Category> expression)
    {
        auto evaluator = RootEvaluator<Kind, Category>(*search->task, expression);
        evaluator.initialize(invocation_context);
        auto invalid = kr::dl::semantics::incremental::Delta {};
        for (const auto index : { a, b })
        {
            const auto object = ygg::make_view(index, *search->task->get_repository());
            if constexpr (std::same_as<Category, kr::dl::ConceptTag>)
                invalid.added.concept_registers.emplace_back(kr::dl::RegisterIdentifier<Category>(0), object);
            else
                invalid.added.role_registers.emplace_back(kr::dl::RegisterIdentifier<Category>(0),
                                                          std::pair(object, ygg::make_view(c, *search->task->get_repository())));
        }
        EXPECT_THROW(evaluator.update(invocation_context, invalid, workspace), std::invalid_argument);
        // The failed root is initialized, not updated, in the next state.
        EXPECT_THROW(evaluator.update(invocation_context, empty_delta, workspace).get_delta(), std::logic_error);
        EXPECT_EQ(evaluator.initialize(invocation_context).size(), 0);

        auto demand = kr::dl::semantics::incremental::IncrementalEvaluation<Kind>(*search->task, { expression });
        const auto root = demand.graph().get_index(expression);
        EXPECT_NO_THROW(demand.enter(invocation_context).evaluate(root));
        auto failed = demand.enter(invocation_context, invalid, workspace);
        EXPECT_THROW(failed.evaluate(root), std::invalid_argument);
        EXPECT_THROW(demand.enter(invocation_context, empty_delta, workspace).delta(root), std::logic_error);
        EXPECT_EQ(demand.enter(invocation_context).evaluate(root).count(), 0);
        auto unchanged = demand.enter(invocation_context, empty_delta, workspace);
        EXPECT_EQ(unchanged.evaluate(root).count(), 0);
        EXPECT_NO_THROW(unchanged.delta(root));  // Updated from the delta.

        const auto roots = std::array<kr::dl::semantics::incremental::EvaluationRoot, 1> { expression };
        auto policy = kr::dl::semantics::DeltaEvaluationPolicy<Kind>(*search->task, roots);
        auto context = policy.make_source_context(invocation_context);
        EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(expression, context).count(), 0);
        auto recovered = policy.make_source_context(invocation_context);
        EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(expression, recovered).count(), 0);
    };
    reject_multiple_values(source);
    reject_multiple_values(kr::ps::ext::dl::parse_role("(r_register 0)", domain, *constructors));
}
}  // namespace

TEST(RunirKrDlIncrementalEvaluation, GroundFeaturesMatchFullEvaluationAndReverse) { check_incremental_evaluation<tyr::GroundTag>(); }
TEST(RunirKrDlIncrementalEvaluation, LiftedFeaturesMatchFullEvaluationAndReverse) { check_incremental_evaluation<tyr::LiftedTag>(); }

}  // namespace runir::tests
