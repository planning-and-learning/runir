#include "planning_fixtures.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <filesystem>
#include <gtest/gtest.h>
#include <iterator>
#include <ranges>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/evaluation_policy.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/dl/semantics/incremental/evaluation.hpp>
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
namespace dl = kr::dl;
namespace sem = dl::semantics;
namespace parser = kr::ps::ext::dl;
using Ext = kr::ExtFamilyTag;
using Object = ygg::Index<tyr::formalism::Object>;

template<typename Index>
concept HasGraphDelta = requires(const sem::incremental::EvaluationGraph<Ext, tyr::GroundTag>& graph, Index index) { graph.get_delta(index); };

template<typename Graph>
concept HasPublicPreparation = requires(Graph& graph, dl::FamilyConstructorView<Ext, dl::ConceptTag> expression) { graph.prepare(expression); };

static_assert(!std::default_initializable<sem::incremental::EvaluationGraph<Ext, tyr::GroundTag>>);
static_assert(!std::copy_constructible<sem::incremental::EvaluationGraph<Ext, tyr::GroundTag>>);
static_assert(std::move_constructible<sem::incremental::EvaluationGraph<Ext, tyr::GroundTag>>);
static_assert(!HasPublicPreparation<sem::incremental::EvaluationGraph<Ext, tyr::GroundTag>>);
static_assert(!std::constructible_from<sem::incremental::Evaluator<Ext, tyr::GroundTag, dl::ConceptTag>, dl::FamilyConstructorView<Ext, dl::ConceptTag>>);

static_assert(!std::constructible_from<sem::incremental::EvaluationIndex<dl::RoleTag>, sem::incremental::EvaluationIndex<dl::ConceptTag>>);
static_assert(!std::constructible_from<sem::incremental::QueryEvaluationIndex, sem::incremental::EvaluationIndex<dl::ConceptTag>>);
static_assert(HasGraphDelta<sem::incremental::EvaluationIndex<dl::ConceptTag>>);
static_assert(HasGraphDelta<sem::incremental::EvaluationIndex<dl::RoleTag>>);
static_assert(HasGraphDelta<sem::incremental::QueryEvaluationIndex>);
static_assert(!HasGraphDelta<sem::incremental::EvaluationIndex<dl::NumericalTag>>);
static_assert(!HasGraphDelta<sem::incremental::EvaluationIndex<dl::BooleanTag>>);
static_assert(requires(const sem::incremental::EvaluationGraph<Ext, tyr::GroundTag>& graph,
                       sem::incremental::EvaluationIndex<dl::ConceptTag> concept_index,
                       sem::incremental::EvaluationIndex<dl::RoleTag> role_index) {
    { graph.get_result(concept_index) } -> std::same_as<sem::BorrowedDenotationView<dl::ConceptTag>>;
    { graph.get_result(role_index) } -> std::same_as<sem::BorrowedDenotationView<dl::RoleTag>>;
});

template<dl::ConceptOrRoleTag Category>
auto elements(const auto& denotation)
{
    auto result = sem::DenotationElementViewList<Category> {};
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

template<dl::ConceptOrRoleTag Category, tyr::formalism::FactKind Fact>
auto negative_atomic(dl::FamilyConstructorView<Ext, Category> positive, dl::ConstructorRepositoryFor<Ext>& repository)
{
    if constexpr (std::same_as<Category, dl::ConceptTag>)
    {
        auto data = positive.get_variant().template get<ygg::Index<dl::Concept<Ext, dl::AtomicStateTag<Fact>>>>().get_data();
        data.polarity = false;
        auto wrapper = ygg::Data<dl::Constructor<Ext, Category>>(dl::insert(repository, data).first.get_index());
        return dl::insert(repository, wrapper).first;
    }
    else
    {
        auto data = positive.get_variant().template get<ygg::Index<dl::Role<Ext, dl::AtomicStateTag<Fact>>>>().get_data();
        data.polarity = false;
        auto wrapper = ygg::Data<dl::Constructor<Ext, Category>>(dl::insert(repository, data).first.get_index());
        return dl::insert(repository, wrapper).first;
    }
}

template<dl::FamilyTag Family, tyr::TaskKind Kind>
void check_planning_graph(tyr::planning::StateView<Kind> source, tyr::planning::StateView<Kind> target)
{
    const auto& task = source.get_task();
    auto constructors = dl::ConstructorRepositoryFactoryFor<Family>().create(task.get_repository());
    auto denotations = sem::DenotationRepositoryFactory().create(task.get_repository());
    auto storage = sem::EvaluationStorage<Family>(denotations);
    auto builder = sem::Builder {};
    auto delta = sem::incremental::Delta<Family> {};
    delta.template assign<Kind>(source, target);
    auto column = ygg::Data<dl::QueryColumn> {};
    column.name = "x";
    const auto x = dl::insert(*constructors, column).first.get_index();
    const auto check = [&]<tyr::formalism::FactKind Fact>()
    {
        for (const auto predicate : task.get_domain().get_domain().template get_predicates<Fact>())
        {
            if (predicate.get_arity() != 1)
                continue;
            auto atom = ygg::Data<dl::Concept<Family, dl::AtomicStateTag<Fact>>>(predicate.get_index());
            auto concept_data = ygg::Data<dl::Constructor<Family, dl::ConceptTag>>(dl::insert(*constructors, atom).first.get_index());
            const auto concept_ = dl::insert(*constructors, concept_data).first;
            auto query_data = ygg::Data<dl::Query<Family, dl::QueryConceptTag>> {};
            query_data.arg = concept_.get_index();
            query_data.columns.push_back(x);
            auto query_wrapper = ygg::Data<dl::Query<Family>>(dl::insert(*constructors, query_data).first.get_index());
            const auto query = dl::insert(*constructors, query_wrapper).first;
            auto count_data = ygg::Data<dl::Numerical<Family, dl::CountTag>>(query.get_index());
            auto count_wrapper = ygg::Data<dl::Constructor<Family, dl::NumericalTag>>(dl::insert(*constructors, count_data).first.get_index());
            const auto count = dl::insert(*constructors, count_wrapper).first;
            auto graph = sem::incremental::EvaluationGraph<Family, Kind>(task, { count });
            const auto concept_id = graph.get_index(concept_);
            const auto count_id = graph.get_index(count);
            const auto compare = [&](auto state)
            {
                storage.reset_dynamic();
                auto context = sem::StateEvaluationContext<Family, Kind>(state, builder, storage);
                EXPECT_EQ(elements<dl::ConceptTag>(graph.get_result(concept_id)), elements<dl::ConceptTag>(sem::evaluate<Kind>(concept_, context)));
                EXPECT_EQ(graph.get_result(count_id).get(), sem::evaluate<Kind>(count, context).get());
            };
            storage.reset_all();
            auto context = sem::StateEvaluationContext<Family, Kind>(source, builder, storage);
            graph.initialize(context);
            compare(source);
            graph.update(delta, builder.get_workspace().get_database_workspace());
            compare(target);
            auto reversed = delta;
            reversed.reverse();
            graph.update(reversed, builder.get_workspace().get_database_workspace());
            compare(source);
            graph.update(sem::incremental::Delta<Family> {}, builder.get_workspace().get_database_workspace());
            EXPECT_FALSE(graph.changed(count_id));
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
    check_planning_graph<kr::BaseFamilyTag, Kind>(nodes.front().get_state(), nodes.back().get_state());
    check_planning_graph<kr::UnsFamilyTag, Kind>(nodes.front().get_state(), nodes.back().get_state());
    const auto domain = search->task->get_domain().get_domain();
    const auto a = domain.get_constants()[0].get_index();
    const auto b = domain.get_constants()[1].get_index();
    const auto c = domain.get_constants()[2].get_index();
    auto constructors = dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<Ext>(denotations);
    auto builder = sem::Builder {};
    auto workspace = ygg::database::Workspace<Object> {};
    auto register_data = std::array<ygg::Data<sem::RegisterValues>, 3> {};
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
    auto argument_concept = ygg::Builder<sem::Denotation<dl::ConceptTag>>(3);
    auto argument_role = ygg::Builder<sem::Denotation<dl::RoleTag>>(3);
    auto argument_boolean = ygg::Builder<sem::Denotation<dl::BooleanTag>>(true);
    auto argument_numerical = ygg::Builder<sem::Denotation<dl::NumericalTag>>(7);
    argument_concept.get().set(ygg::uint_t(a));
    argument_role.get(a).set(ygg::uint_t(b));
    auto argument_data = ygg::Data<sem::CallArguments> {};
    argument_data.concept_arguments.push_back(sem::insert(denotations, argument_concept, builder).first.get_index());
    argument_data.role_arguments.push_back(sem::insert(denotations, argument_role, builder).first.get_index());
    argument_data.boolean_arguments.push_back(sem::insert(denotations, argument_boolean, builder).first.get_index());
    argument_data.numerical_arguments.push_back(sem::insert(denotations, argument_numerical, builder).first.get_index());
    const auto arguments = sem::insert(denotations, argument_data).first;
    argument_concept.get().reset(ygg::uint_t(a));
    argument_concept.get().set(ygg::uint_t(c));
    argument_role.get(a).reset(ygg::uint_t(b));
    argument_role.get(c).set(ygg::uint_t(a));
    argument_boolean.value = false;
    argument_numerical.value = 2;
    argument_data.concept_arguments[0] = sem::insert(denotations, argument_concept, builder).first.get_index();
    argument_data.role_arguments[0] = sem::insert(denotations, argument_role, builder).first.get_index();
    argument_data.boolean_arguments[0] = sem::insert(denotations, argument_boolean, builder).first.get_index();
    argument_data.numerical_arguments[0] = sem::insert(denotations, argument_numerical, builder).first.get_index();
    const auto other_arguments = sem::insert(denotations, argument_data).first;
    const auto context_for = [&](auto state, size_t i, sem::CallArgumentsView invocation)
    { return sem::StateEvaluationContext<Ext, Kind, decltype(state), sem::BorrowedRegisterValuesView>(state, builder, storage, invocation, registers(i)); };
    auto deltas = std::array<sem::incremental::Delta<Ext>, 2> {};
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
    const auto empty_delta = sem::incremental::Delta<Ext> {};
    const auto expect_unpublished = [&]
    {
        const auto& repository = storage.get_denotation_repository(false);
        EXPECT_EQ(repository.template size<sem::Denotation<dl::ConceptTag>>(), 0);
        EXPECT_EQ(repository.template size<sem::Denotation<dl::RoleTag>>(), 0);
        EXPECT_EQ(repository.template size<sem::Denotation<dl::BooleanTag>>(), 0);
        EXPECT_EQ(repository.template size<sem::Denotation<dl::NumericalTag>>(), 0);
        EXPECT_TRUE(repository.get_relation_repository().empty());
        EXPECT_TRUE(storage.get_caches().template get<dl::ConceptTag>(false).empty());
        EXPECT_TRUE(storage.get_caches().template get<dl::RoleTag>(false).empty());
        EXPECT_TRUE(storage.get_caches().template get<dl::BooleanTag>(false).empty());
        EXPECT_TRUE(storage.get_caches().template get<dl::NumericalTag>(false).empty());
    };

    const auto check = [&]<dl::CategoryTag Category>(dl::FamilyConstructorView<Ext, Category> expression)
    {
        auto evaluator = sem::incremental::Evaluator<Ext, Kind, Category>(*search->task, expression);
        EXPECT_THROW(evaluator.update(empty_delta, workspace), std::logic_error);
        const auto snapshot = [&]
        {
            if constexpr (dl::ConceptOrRoleTag<Category>)
                return elements<Category>(evaluator.get_result());
            else
                return evaluator.get_result().get();
        };
        const auto expect_empty_delta = [&]
        {
            if constexpr (dl::ConceptOrRoleTag<Category>)
            {
                EXPECT_TRUE(evaluator.get_delta().added.empty());
                EXPECT_TRUE(evaluator.get_delta().removed.empty());
            }
            else
                EXPECT_FALSE(evaluator.changed());
        };
        const auto compare_full = [&](auto& context)
        {
            const auto expected = sem::evaluate<Kind>(expression, context);
            if constexpr (dl::ConceptOrRoleTag<Category>)
            {
                EXPECT_EQ(snapshot(), elements<Category>(expected));
                EXPECT_EQ(evaluator.size(), snapshot().size());
                if constexpr (std::same_as<Category, dl::ConceptTag>)
                    EXPECT_EQ(evaluator.get_result().get().count(), evaluator.size());
                else
                    EXPECT_EQ(evaluator.get_result().count(), evaluator.size());
            }
            else
                EXPECT_EQ(snapshot(), expected.get());
        };
        storage.reset_all();
        auto initial_context = context_for(nodes.front().get_state(), 0, arguments);
        evaluator.initialize(initial_context);
        const auto retained_view = evaluator.get_result();
        const ygg::uint_t* blocks = nullptr;
        if constexpr (dl::ConceptOrRoleTag<Category>)
            blocks = retained_view.get_handle().blocks.data();
        expect_empty_delta();
        expect_unpublished();
        compare_full(initial_context);
        const auto update = [&](const auto& delta, size_t i)
        {
            const auto before = snapshot();
            storage.reset_dynamic();
            evaluator.update(delta, workspace);
            expect_unpublished();
            if constexpr (dl::ConceptOrRoleTag<Category>)
            {
                EXPECT_EQ(evaluator.get_result().get_handle().blocks.data(), blocks);
                EXPECT_EQ(elements<Category>(retained_view), snapshot());
                EXPECT_TRUE(std::ranges::is_permutation(evaluator.get_delta().added, difference(snapshot(), before)));
                EXPECT_TRUE(std::ranges::is_permutation(evaluator.get_delta().removed, difference(before, snapshot())));
            }
            else
            {
                EXPECT_EQ(retained_view.get(), snapshot());
                EXPECT_EQ(evaluator.changed(), before != snapshot());
            }
            auto context = context_for(nodes[i].get_state(), i, arguments);
            compare_full(context);
            const auto after = snapshot();
            evaluator.update(empty_delta, workspace);
            expect_empty_delta();
            EXPECT_EQ(snapshot(), after);
        };
        for (size_t i = 0; i < deltas.size(); ++i)
            update(deltas[i], i + 1);
        for (size_t i = deltas.size(); i > 0; --i)
        {
            auto reversed = deltas[i - 1];
            reversed.reverse();
            update(reversed, i - 1);
        }

        // Resetting repositories cannot invalidate borrowed evaluator results. Reinitializing
        // on the same task retains static results but reloads invocation arguments.
        const auto before_reset = snapshot();
        storage.reset_all();
        EXPECT_EQ(snapshot(), before_reset);
        const auto borrowed_builder = nodes.back().get_state().get_state_builder();
        auto final_context = context_for(ygg::make_view(borrowed_builder, *search->task), 2, other_arguments);
        evaluator.initialize(final_context);
        expect_empty_delta();
        expect_unpublished();
        EXPECT_TRUE(storage.get_denotation_repository(true).get_vector_repository().empty());
        EXPECT_TRUE(storage.get_caches().template get<Category>(true).empty());
        compare_full(final_context);
    };

    const auto check_leaf = [&]<dl::ConceptOrRoleTag Category>(dl::FamilyConstructorView<Ext, Category> expression)
    {
        check(expression);
        auto count = ygg::Data<dl::Numerical<Ext, dl::CountTag>>(expression.get_index());
        auto count_wrapper = ygg::Data<dl::Constructor<Ext, dl::NumericalTag>>(dl::insert(*constructors, count).first.get_index());
        check(dl::insert(*constructors, count_wrapper).first);
        auto nonempty = ygg::Data<dl::Boolean<Ext, dl::NonemptyTag>>(expression.get_index());
        auto nonempty_wrapper = ygg::Data<dl::Constructor<Ext, dl::BooleanTag>>(dl::insert(*constructors, nonempty).first.get_index());
        check(dl::insert(*constructors, nonempty_wrapper).first);
    };
    const auto present = parser::parse_concept(R"((c_atomic_state "present"))", domain, *constructors);
    const auto copied_present = parser::parse_concept(R"((c_atomic_state "copied-present"))", domain, *constructors);
    const auto edge = parser::parse_role(R"((r_atomic_state "edge"))", domain, *constructors);
    const auto copied_edge = parser::parse_role(R"((r_atomic_state "copied-edge"))", domain, *constructors);
    const auto concept_expressions = std::vector {
        present,
        copied_present,
        negative_atomic<dl::ConceptTag, tyr::formalism::FluentTag>(present, *constructors),
        negative_atomic<dl::ConceptTag, tyr::formalism::DerivedTag>(copied_present, *constructors),
        parser::parse_concept("(c_register 0)", domain, *constructors),
        parser::parse_concept("(c_argument 0)", domain, *constructors),
        parser::parse_concept(R"((c_and (c_atomic_state "fixed") (c_not (c_nominal "a"))))", domain, *constructors),
        parser::parse_concept(R"((c_atomic_goal "present" true))", domain, *constructors),
        parser::parse_concept(R"((c_atomic_goal "present" false))", domain, *constructors),
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
        check_leaf(parser::parse_concept(text, domain, *constructors));
    }
    for (const auto* operation : { "c_at_least", "c_at_most", "c_exactly" })
        for (const auto threshold : { 0, 1, 2 })
        {
            const auto prefix = "(" + std::string(operation) + " " + std::to_string(threshold) + " " + edge_text;
            SCOPED_TRACE(prefix);
            check_leaf(parser::parse_concept(prefix + ")", domain, *constructors));
            check_leaf(parser::parse_concept(prefix + " " + present_text + ")", domain, *constructors));
        }
    const auto role_expressions = std::vector {
        edge,
        copied_edge,
        negative_atomic<dl::RoleTag, tyr::formalism::FluentTag>(edge, *constructors),
        negative_atomic<dl::RoleTag, tyr::formalism::DerivedTag>(copied_edge, *constructors),
        parser::parse_role("(r_register 0)", domain, *constructors),
        parser::parse_role("(r_argument 0)", domain, *constructors),
        parser::parse_role("(r_inverse (r_universal))", domain, *constructors),
        parser::parse_role(R"((r_atomic_goal "edge" true))", domain, *constructors),
        parser::parse_role(R"((r_atomic_goal "edge" false))", domain, *constructors),
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
        check_leaf(parser::parse_role(text, domain, *constructors));
    }
    check(parser::parse_boolean("(b_argument 0)", domain, *constructors));
    check(parser::parse_numerical("(n_argument 0)", domain, *constructors));
    check(parser::parse_numerical(R"((n_count (q_atomic_state "edge" (x y))))", domain, *constructors));
    // Query cardinality follows 1→1→0 and undo. Count and nonempty must report unchanged
    // on replacement/no-op, and reload the empty baseline on reinitialization.
    for (const auto* text : { "(q_concept x (c_register 0))", "(q_role (x y) (r_register 0))" })
    {
        SCOPED_TRACE(text);
        check(parser::parse_numerical("(n_count " + std::string(text) + ")", domain, *constructors));
        check(parser::parse_boolean("(b_nonempty " + std::string(text) + ")", domain, *constructors));
    }
    check(parser::parse_boolean(R"((b_nonempty (q_atomic_state "edge" (x y))))", domain, *constructors));
    for (const auto* predicate : { "ready", "copied-ready" })
        for (const auto* polarity : { "true", "false" })
            check(parser::parse_boolean("(b_atomic_state \"" + std::string(predicate) + "\" " + polarity + ")", domain, *constructors));
    const auto count_present = "(n_count " + present_text + ")";
    for (const auto* operation : { "n_add", "n_sub", "n_mul", "n_div", "n_min", "n_max" })
    {
        const auto text = "(" + std::string(operation) + " " + count_present + " (n_argument 0))";
        SCOPED_TRACE(text);
        check(parser::parse_numerical(text, domain, *constructors));
    }
    const auto distance = "(n_distance (c_register 0) " + edge_text + " (c_register 1))";
    const auto additional_numericals = std::vector<std::string> {
        "(n_const 0)",
        "(n_distance " + present_text + " " + edge_text + " (c_nominal \"c\"))",
        "(n_distance " + present_text + " " + edge_text + " " + present_text + ")",
        distance,
        "(n_add " + distance + " (n_const 1))",
        "(n_div (n_count " + edge_text + ") " + count_present + ")",
    };
    for (const auto& text : additional_numericals)
    {
        SCOPED_TRACE(text);
        check(parser::parse_numerical(text, domain, *constructors));
    }

    // Module features share their operands across counts, distances, and query conversions.
    const auto source = parser::parse_concept("(c_register 0)", domain, *constructors);
    const auto target = parser::parse_concept("(c_register 1)", domain, *constructors);
    const auto source_count = parser::parse_numerical("(n_count (c_register 0))", domain, *constructors);
    const auto edge_count = parser::parse_numerical("(n_count " + edge_text + ")", domain, *constructors);
    const auto distance_expression = parser::parse_numerical(distance, domain, *constructors);
    const auto self_distance = parser::parse_numerical("(n_distance (c_register 0) " + edge_text + " (c_register 0))", domain, *constructors);
    const auto query_count = parser::parse_numerical("(n_count (q_role (x y) " + edge_text + "))", domain, *constructors);
    const auto query =
        query_count.get_variant().template get<ygg::Index<dl::Numerical<Ext, dl::CountTag>>>().get_arg().template get<ygg::Index<dl::Query<Ext>>>();
    // Duplicate roots and explicitly requested shared children do not duplicate nodes.
    auto graph = sem::incremental::EvaluationGraph<Ext, Kind>(
        *search->task,
        { source_count, edge_count, distance_expression, self_distance, query_count, query, distance_expression, source, edge });
    const auto source_id = graph.get_index(source);
    const auto edge_id = graph.get_index(edge);
    const auto target_id = graph.get_index(target);  // A prepared child can be inspected without making it another root.
    const auto source_count_id = graph.get_index(source_count);
    const auto edge_count_id = graph.get_index(edge_count);
    const auto distance_id = graph.get_index(distance_expression);
    const auto self_distance_id = graph.get_index(self_distance);
    const auto query_id = graph.get_index(query);
    const auto query_count_id = graph.get_index(query_count);
    EXPECT_EQ(graph.node_count(), 9);
    EXPECT_EQ(graph.get_index(distance_expression), distance_id);
    EXPECT_EQ(graph.get_index(source), source_id);
    EXPECT_EQ(graph.get_index(edge), edge_id);
    EXPECT_EQ(graph.get_index(query), query_id);
    const auto unprepared = parser::parse_numerical("(n_const 42)", domain, *constructors);
    EXPECT_THROW(graph.get_index(unprepared), std::invalid_argument);
    EXPECT_EQ(graph.node_count(), 9);
    const auto scalar_expressions = std::array { source_count, edge_count, distance_expression, self_distance, query_count };
    const auto scalar_ids = std::array { source_count_id, edge_count_id, distance_id, self_distance_id, query_count_id };
    const auto scalar_values = [&]
    {
        auto values = std::array<ygg::uint_t, 5> {};
        for (size_t i = 0; i < values.size(); ++i)
            values[i] = graph.get_result(scalar_ids[i]).get();
        return values;
    };
    const auto query_rows = [](const auto& relation)
    {
        auto result = std::vector<std::vector<Object>> {};
        for (size_t i = 0; i < relation.size(); ++i)
        {
            const auto row = relation.row(i);
            result.emplace_back(row.begin(), row.end());
        }
        std::ranges::sort(result);
        return result;
    };
    const auto compare_graph = [&](auto& context)
    {
        EXPECT_EQ(elements<dl::ConceptTag>(graph.get_result(source_id)), elements<dl::ConceptTag>(sem::evaluate<Kind>(source, context)));
        EXPECT_EQ(elements<dl::ConceptTag>(graph.get_result(target_id)), elements<dl::ConceptTag>(sem::evaluate<Kind>(target, context)));
        EXPECT_EQ(elements<dl::RoleTag>(graph.get_result(edge_id)), elements<dl::RoleTag>(sem::evaluate<Kind>(edge, context)));
        for (size_t i = 0; i < scalar_ids.size(); ++i)
            EXPECT_EQ(graph.get_result(scalar_ids[i]).get(), sem::evaluate<Kind>(scalar_expressions[i], context).get());
        EXPECT_EQ(query_rows(graph.get_result(query_id)), query_rows(sem::evaluate<Kind>(query, context)));
    };
    storage.reset_all();
    auto initial_context = context_for(nodes.front().get_state(), 0, arguments);
    EXPECT_THROW(graph.get_result(source_id), std::logic_error);
    EXPECT_THROW(graph.get_delta(query_id), std::logic_error);
    // Copying the task preserves the formalism repository but changes task identity.
    const auto other_task = *search->task;
    const auto borrowed_builder = nodes.front().get_state().get_state_builder();
    auto foreign_context = context_for(ygg::make_view(borrowed_builder, other_task), 0, arguments);
    const auto policy_roots = std::array<sem::incremental::EvaluationRoot<Ext>, 1> { source_count };
    auto policy = kr::dl::semantics::DeltaEvaluationPolicy<Ext, Kind>(*search->task, policy_roots);
    EXPECT_NO_THROW(policy.make_source_context(initial_context));
    EXPECT_THROW(policy.make_source_context(foreign_context), std::invalid_argument);
    policy.reset_source();
    EXPECT_THROW(policy.make_source_context(foreign_context), std::invalid_argument);
    EXPECT_NO_THROW(policy.make_source_context(initial_context));
    EXPECT_THROW(graph.initialize(foreign_context), std::invalid_argument);
    EXPECT_THROW(graph.get_result(source_id), std::logic_error);
    graph.initialize(initial_context);
    EXPECT_THROW(graph.initialize(foreign_context), std::invalid_argument);
    // Rejection must preserve the valid baseline and its prepared handles.
    EXPECT_EQ(graph.get_index(source), source_id);
    expect_unpublished();
    compare_graph(initial_context);
    const auto source_view = graph.get_result(source_id);
    const auto edge_view = graph.get_result(edge_id);
    const auto* source_blocks = source_view.get_handle().blocks.data();
    const auto* edge_blocks = edge_view.get_handle().blocks.data();
    const auto update_graph = [&](const auto& delta, size_t i)
    {
        const auto before_source = elements<dl::ConceptTag>(source_view);
        const auto before_edge = elements<dl::RoleTag>(edge_view);
        const auto before_query = query_rows(graph.get_result(query_id));
        const auto before_scalars = scalar_values();
        storage.reset_dynamic();
        graph.update(delta, workspace);
        expect_unpublished();
        EXPECT_EQ(source_view.get_handle().blocks.data(), source_blocks);
        EXPECT_EQ(edge_view.get_handle().blocks.data(), edge_blocks);
        const auto after_source = elements<dl::ConceptTag>(source_view);
        const auto after_edge = elements<dl::RoleTag>(edge_view);
        EXPECT_TRUE(std::ranges::is_permutation(graph.get_delta(source_id).added, difference(after_source, before_source)));
        EXPECT_TRUE(std::ranges::is_permutation(graph.get_delta(source_id).removed, difference(before_source, after_source)));
        EXPECT_TRUE(std::ranges::is_permutation(graph.get_delta(edge_id).added, difference(after_edge, before_edge)));
        EXPECT_TRUE(std::ranges::is_permutation(graph.get_delta(edge_id).removed, difference(before_edge, after_edge)));
        EXPECT_EQ(query_rows(graph.get_delta(query_id).added), difference(query_rows(graph.get_result(query_id)), before_query));
        EXPECT_EQ(query_rows(graph.get_delta(query_id).removed), difference(before_query, query_rows(graph.get_result(query_id))));
        for (size_t j = 0; j < scalar_ids.size(); ++j)
            EXPECT_EQ(graph.changed(scalar_ids[j]), before_scalars[j] != scalar_values()[j]);
        auto context = context_for(nodes[i].get_state(), i, arguments);
        compare_graph(context);
        const auto after_scalars = scalar_values();
        graph.update(empty_delta, workspace);
        EXPECT_TRUE(graph.get_delta(source_id).added.empty());
        EXPECT_TRUE(graph.get_delta(source_id).removed.empty());
        EXPECT_TRUE(graph.get_delta(edge_id).added.empty());
        EXPECT_TRUE(graph.get_delta(edge_id).removed.empty());
        EXPECT_TRUE(graph.get_delta(query_id).added.empty());
        EXPECT_TRUE(graph.get_delta(query_id).removed.empty());
        for (const auto id : scalar_ids)
            EXPECT_FALSE(graph.changed(id));
        EXPECT_EQ(scalar_values(), after_scalars);
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
    graph.initialize(invocation_context);
    expect_unpublished();
    for (const auto id : scalar_ids)
        EXPECT_FALSE(graph.changed(id));
    compare_graph(invocation_context);

    // This invocation has empty registers. A failed update invalidates every root,
    // even those whose child results were already visited before the failure.
    auto invalid_delta = sem::incremental::Delta<Ext> {};
    invalid_delta.removed.concept_registers.emplace_back(dl::RegisterIdentifier<dl::ConceptTag>(0), ygg::make_view(a, *search->task->get_repository()));
    EXPECT_THROW(graph.update(invalid_delta, workspace), std::invalid_argument);
    EXPECT_THROW(graph.get_result(source_id), std::logic_error);
    EXPECT_THROW(graph.get_result(query_id), std::logic_error);
    EXPECT_THROW(graph.update(empty_delta, workspace), std::logic_error);
    graph.initialize(invocation_context);
    compare_graph(invocation_context);

    const auto reject_multiple_values = [&]<dl::ConceptOrRoleTag Category>(dl::FamilyConstructorView<Ext, Category> expression)
    {
        auto evaluator = sem::incremental::Evaluator<Ext, Kind, Category>(*search->task, expression);
        evaluator.initialize(invocation_context);
        auto invalid = sem::incremental::Delta<Ext> {};
        for (const auto index : { a, b })
        {
            const auto object = ygg::make_view(index, *search->task->get_repository());
            if constexpr (std::same_as<Category, dl::ConceptTag>)
                invalid.added.concept_registers.emplace_back(dl::RegisterIdentifier<Category>(0), object);
            else
                invalid.added.role_registers.emplace_back(dl::RegisterIdentifier<Category>(0),
                                                          std::pair(object, ygg::make_view(c, *search->task->get_repository())));
        }
        EXPECT_THROW(evaluator.update(invalid, workspace), std::invalid_argument);
        EXPECT_THROW(evaluator.get_result(), std::logic_error);
        evaluator.initialize(invocation_context);
        EXPECT_EQ(evaluator.size(), 0);
    };
    reject_multiple_values(source);
    reject_multiple_values(parser::parse_role("(r_register 0)", domain, *constructors));
}
}  // namespace

TEST(RunirKrDlIncrementalEvaluation, GroundFeaturesMatchFullEvaluationAndReverse) { check_incremental_evaluation<tyr::GroundTag>(); }
TEST(RunirKrDlIncrementalEvaluation, LiftedFeaturesMatchFullEvaluationAndReverse) { check_incremental_evaluation<tyr::LiftedTag>(); }

}  // namespace runir::tests
