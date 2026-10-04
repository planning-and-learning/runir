#include "planning_fixtures.hpp"

#include <algorithm>
#include <array>
#include <cista/serialization.h>
#include <filesystem>
#include <gtest/gtest.h>
#include <limits>
#include <optional>
#include <runir/kr/dl/query_data.hpp>
#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/evaluation.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/database/operations.hpp>
#include <yggdrasil/execution/onetbb.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace runir::tests
{
namespace
{
namespace dl = kr::dl;
namespace sem = dl::semantics;
namespace parser = kr::ps::ext::dl;
using Ext = kr::ExtFamilyTag;
using ObjectIndex = ygg::Index<tyr::formalism::Object>;
using ColumnIndex = ygg::Index<ygg::database::Column>;
static_assert(!std::same_as<ColumnIndex, ygg::Index<tyr::formalism::Object>>);
static_assert(!std::is_convertible_v<ygg::Index<tyr::formalism::Object>, ColumnIndex>);

template<tyr::TaskKind Kind>
auto query_search_context()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    if constexpr (std::same_as<Kind, tyr::GroundTag>)
        return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    else
        return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
}

auto parse_query(const std::string& expression, tyr::formalism::planning::DomainView domain, dl::ConstructorRepositoryFor<Ext>& repository)
{
    const auto count = parser::parse_numerical("(n_count " + expression + ")", domain, repository);
    return count.get_variant().template get<ygg::Index<dl::Numerical<Ext, dl::CountTag>>>().get_arg().template get<ygg::Index<dl::Query<Ext>>>();
}

template<typename Data, typename Check>
void check_relocated_data(const Data& data, Check check)
{
    auto relocated = [&]
    {
        const auto original = cista::serialize(data);
        auto copy = original;
        EXPECT_NE(copy.data(), original.data());
        return copy;
    }();
    const auto* decoded = cista::deserialize<Data>(relocated);
    EXPECT_EQ(*decoded, data);
    EXPECT_EQ(decoded->index, data.index);
    check(*decoded);
}

template<typename Context>
auto persistent_context(Context& source, sem::DenotationCaches<Ext>& caches, sem::DenotationRepository& repository, sem::EvaluationStorage<Ext>& intermediates)
{
    return sem::StateEvaluationContext<Ext,
                                       typename Context::KindType,
                                       std::remove_cvref_t<decltype(source.get_state())>,
                                       std::remove_cvref_t<decltype(source.registers())>>(source.get_state(),
                                                                                          source.get_builder(),
                                                                                          caches,
                                                                                          repository,
                                                                                          intermediates,
                                                                                          source.arguments(),
                                                                                          source.registers());
}

template<tyr::TaskKind Kind>
void check_queries()
{
    const auto search = query_search_context<Kind>();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = sem::Builder();
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<Ext>(denotations);
    auto empty_arguments = ygg::Data<sem::CallArguments>();
    auto registers = ygg::Data<sem::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = sem::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                          builder,
                                                          storage,
                                                          sem::insert(denotations, empty_arguments).first,
                                                          sem::insert(denotations, registers).first);

    const auto triple = std::string(R"((q_atomic_state "triple" (x y z)))");
    const auto fixed = std::string(R"((q_atomic_state "fixed" (x y z)))");
    const auto ready = std::string(R"((q_atomic_state "ready" ()))");
    const auto missing = std::string(R"((q_atomic_state "missing" ()))");
    const auto count = [&](const std::string& query)
    {
        const auto expression = parser::parse_numerical("(n_count " + query + ")", domain, *repository);
        return sem::evaluate(expression, context).get();
    };
    const std::vector<std::pair<std::string, ygg::uint_t>> cases {
        { triple, 4 },
        { fixed, 2 },
        { R"((q_atomic_state "copied" (x y z)))", 4 },
        { R"((q_atomic_state "assignment" (x y z w)))", 4 },
        { R"((q_project (x) (q_join (q_atomic_state "assignment" (x y z w)) (q_atomic_state "required" (x y z)))))", 1 },
        { ready, 1 },
        { missing, 0 },
        { R"((q_atomic_goal "triple" true (x y z)))", 1 },
        { R"((q_atomic_goal "triple" false (x y z)))", 1 },
        { "(q_project (x) " + triple + ")", 2 },
        { "(q_project (y x) " + triple + ")", 3 },
        { "(q_project () " + triple + ")", 1 },
        { "(q_project () " + missing + ")", 0 },
        { "(q_select_equal y z " + triple + ")", 2 },
        { "(q_select_equal x y " + triple + ")", 1 },
        { "(q_select_value x \"a\" " + triple + ")", 2 },
        { "(q_join " + triple + " " + fixed + ")", 1 },
        { "(q_union " + triple + " " + fixed + ")", 5 },
        { "(q_difference " + triple + " " + fixed + ")", 3 },
        { "(q_join " + triple + " " + ready + ")", 4 },
        { "(q_join " + triple + " " + missing + ")", 0 },
        { "(q_join (q_rename (u v w) " + triple + R"() (q_atomic_state "marker" (u))))", 2 },
        { "(q_join " + triple + R"( (q_concept x (c_nominal "a"))))", 2 },
        { "(q_join " + triple + R"( (q_role (x y) (r_atomic_state "edge"))))", 3 },
        { R"((q_join (q_join (q_atomic_state "marker" (a)) (q_atomic_state "marker" (b)))
                        (q_join (q_atomic_state "marker" (c)) (q_atomic_state "marker" (d)))))",
          16 },
        { "(q_concept x (c_project z " + triple + "))", 3 },
    };
    // Reevaluate different schemas/arity and empty outputs using the retained workspace.
    for (int repeat = 0; repeat < 3; ++repeat)
    {
        storage.reset_all();
        for (const auto& [query, expected] : cases)
        {
            SCOPED_TRACE(query);
            EXPECT_EQ(count(query), expected);
            EXPECT_EQ(count(query), expected);
        }
    }

    EXPECT_TRUE(sem::evaluate(parser::parse_boolean("(b_nonempty " + ready + ")", domain, *repository), context).get());
    EXPECT_FALSE(sem::evaluate(parser::parse_boolean("(b_nonempty " + missing + ")", domain, *repository), context).get());
    const auto concept_ = parser::parse_concept("(c_project z " + triple + ")", domain, *repository);
    EXPECT_EQ(sem::evaluate(concept_, context).get().count(), 3);
    const auto role = parser::parse_role("(r_project y x " + triple + ")", domain, *repository);
    const auto reversed = sem::evaluate(role, context);
    EXPECT_EQ(reversed.count(), 3);
    const auto a = domain.get_constants()[0];
    const auto b = domain.get_constants()[1];
    const auto c = domain.get_constants()[2];
    ASSERT_EQ(a.get_name(), "a");
    ASSERT_EQ(b.get_name(), "b");
    ASSERT_EQ(c.get_name(), "c");
    EXPECT_TRUE(reversed.get(b.get_index())[ygg::uint_t(a.get_index())]);
    EXPECT_TRUE(reversed.get(c.get_index())[ygg::uint_t(b.get_index())]);
    EXPECT_FALSE(reversed.get(a.get_index())[ygg::uint_t(b.get_index())]);

    // a has a matching y and a matching z in different rows; only b has one complete witness.
    const auto correlated =
        parser::parse_concept(R"((c_project x (q_join (q_atomic_state "assignment" (x y z w)) (q_atomic_state "required" (x y z)))))", domain, *repository);
    const auto witnesses = sem::evaluate(correlated, context).get();
    EXPECT_EQ(witnesses.count(), 1);
    EXPECT_TRUE(witnesses[ygg::uint_t(b.get_index())]);
    EXPECT_FALSE(witnesses[ygg::uint_t(a.get_index())]);

    const auto evaluate_register = [&](auto expression)
    {
        auto updated = sem::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                              builder,
                                                              storage,
                                                              sem::insert(denotations, empty_arguments).first,
                                                              sem::insert(denotations, registers).first);
        return sem::evaluate(expression, updated);
    };
    const auto register_count = parser::parse_numerical("(n_count (q_join " + triple + " (q_concept x (c_register 0))))", domain, *repository);
    EXPECT_EQ(evaluate_register(register_count).get(), 0);
    registers.concept_values[0] = a.get_index();
    storage.reset_dynamic();
    EXPECT_EQ(evaluate_register(register_count).get(), 2);
    registers.concept_values[0] = c.get_index();
    storage.reset_dynamic();
    EXPECT_EQ(evaluate_register(register_count).get(), 0);
    registers.concept_values[0].reset();
    storage.reset_dynamic();
    EXPECT_EQ(evaluate_register(register_count).get(), 0);

    const auto role_register_count = parser::parse_numerical("(n_count (q_join " + triple + " (q_role (x y) (r_register 0))))", domain, *repository);
    registers.role_values[0] = ::cista::pair(a.get_index(), b.get_index());
    storage.reset_dynamic();
    EXPECT_EQ(evaluate_register(role_register_count).get(), 2);
    registers.role_values[0] = ::cista::pair(b.get_index(), c.get_index());
    storage.reset_dynamic();
    EXPECT_EQ(evaluate_register(role_register_count).get(), 1);

    EXPECT_FALSE(context.registers().template get<dl::ConceptTag>()[0]);
    EXPECT_FALSE(context.registers().template get<dl::RoleTag>()[0]);

    auto persistent_memo = sem::DenotationCaches<Ext> {};
    auto persistent = persistent_context(context, persistent_memo, denotations, storage);
    const auto a_set = sem::evaluate(parser::parse_concept(R"((c_nominal "a"))", domain, *repository), persistent);
    const auto c_set = sem::evaluate(parser::parse_concept(R"((c_nominal "c"))", domain, *repository), persistent);
    auto argument_values = ygg::Data<sem::CallArguments>();
    argument_values.concept_arguments.push_back(a_set.get_index());
    auto argument_context = sem::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                                   builder,
                                                                   storage,
                                                                   sem::insert(denotations, argument_values).first,
                                                                   sem::insert(denotations, registers).first);
    const auto argument_count = parser::parse_numerical("(n_count (q_join " + triple + " (q_concept x (c_argument 0))))", domain, *repository);
    // An invalid Ext argument must propagate through DL/query variant dispatch, not terminate.
    EXPECT_THROW(sem::evaluate(argument_count, context), std::out_of_range);
    storage.reset_dynamic();
    EXPECT_EQ(sem::evaluate(argument_count, argument_context).get(), 2);
    const auto argument = parser::parse_concept("(c_argument 0)", domain, *repository);
    EXPECT_EQ(sem::evaluate(argument, argument_context), a_set);
    argument_values.concept_arguments[0] = c_set.get_index();
    auto other_argument_context = sem::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                                         builder,
                                                                         storage,
                                                                         sem::insert(denotations, argument_values).first,
                                                                         sem::insert(denotations, registers).first);
    storage.reset_dynamic();
    EXPECT_EQ(sem::evaluate(argument_count, other_argument_context).get(), 0);
    EXPECT_EQ(argument_context.arguments().template get<dl::ConceptTag>()[0].get_index(), a_set.get_index());

    const auto successors = search->successor_generator->get_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator);
    ASSERT_FALSE(successors.empty());
    auto next = sem::StateEvaluationContext<Ext, Kind>(successors.front().get_state(),
                                                       builder,
                                                       storage,
                                                       sem::insert(denotations, empty_arguments).first,
                                                       sem::insert(denotations, registers).first);
    const auto triple_count = parser::parse_numerical("(n_count " + triple + ")", domain, *repository);
    const auto derived_count = parser::parse_numerical(R"((n_count (q_atomic_state "copied" (x y z))))", domain, *repository);
    const auto ready_test = parser::parse_boolean("(b_nonempty " + ready + ")", domain, *repository);
    storage.reset_dynamic();
    EXPECT_EQ(sem::evaluate(triple_count, next).get(), 3);
    EXPECT_EQ(sem::evaluate(derived_count, next).get(), 3);
    EXPECT_FALSE(sem::evaluate(ready_test, next).get());
    storage.reset_dynamic();
    EXPECT_EQ(sem::evaluate(triple_count, context).get(), 4);
    EXPECT_TRUE(sem::evaluate(ready_test, context).get());

    // Borrowed state/register views use interned arguments, which survive feature-cache invalidation.
    {
        auto argument_data = ygg::Data<sem::CallArguments> {};
        argument_data.concept_arguments.push_back(a_set.get_index());
        const auto arguments = sem::insert(denotations, argument_data).first;
        const auto planning_state = initial.get_state();
        auto borrowed_context = sem::StateEvaluationContext<Ext, Kind, tyr::planning::BuilderStateView<Kind>, sem::BorrowedRegisterValuesView>(
            tyr::planning::BuilderStateView<Kind>(planning_state.get_state_builder(), *search->task),
            builder,
            storage,
            arguments,
            ygg::make_view(registers, *search->task->get_repository()));
        static_assert(std::is_same_v<decltype(borrowed_context.arguments()), sem::CallArgumentsView>);
        EXPECT_EQ(&borrowed_context.arguments().get_data(), &arguments.get_data());
        auto copied_context = borrowed_context;
        EXPECT_EQ(&copied_context.arguments().get_data(), &arguments.get_data());
        storage.reset_dynamic();
        EXPECT_EQ(sem::evaluate(argument, borrowed_context), a_set);
        EXPECT_EQ(storage.get_denotation_repository(false).template size<sem::Denotation<dl::ConceptTag>>(), 0);
        storage.reset_dynamic();
        EXPECT_TRUE(a_set.get()[ygg::uint_t(a.get_index())]);
        EXPECT_EQ(sem::evaluate(argument, borrowed_context), a_set);
        storage.reset_dynamic();
    }

    // Argument nodes forward their durable identity, while computed roots use the selected output binding.
    auto destination = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto destination_memo = sem::DenotationCaches<Ext> {};
    auto argument_output = persistent_context(argument_context, destination_memo, destination, storage);
    const auto retained_argument = sem::evaluate(argument, argument_output);
    EXPECT_EQ(retained_argument, a_set);
    EXPECT_EQ(destination.template size<sem::Denotation<dl::ConceptTag>>(), 0);
    const auto distance = parser::parse_numerical(R"((n_distance (c_nominal "a") (r_atomic_state "edge") (c_nominal "c")))", domain, *repository);
    auto computed_output = persistent_context(context, destination_memo, destination, storage);
    const auto retained_distance = sem::evaluate(distance, computed_output);
    EXPECT_EQ(&retained_distance.get_context(), &destination);
    EXPECT_EQ(destination.template size<sem::Denotation<dl::ConceptTag>>(), 0);
    EXPECT_EQ(destination.template size<sem::Denotation<dl::RoleTag>>(), 0);
    storage.reset_all();
    EXPECT_EQ(retained_argument.get().count(), 1);
    EXPECT_TRUE(retained_argument.get()[ygg::uint_t(a.get_index())]);
    denotations.clear();
    EXPECT_EQ(retained_distance.get(), 2);
}

template<dl::FamilyTag Family>
void check_cached_queries()
{
    const auto search = query_search_context<tyr::GroundTag>();
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    auto factory = dl::ConstructorRepositoryFactoryFor<Family>();
    auto repository = factory.create(search->task->get_repository());
    auto other_repository = factory.create(search->task->get_repository());
    auto& repo = *repository;
    auto builder = sem::Builder();
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<Family>(denotations);
    auto& caches = storage.get_caches();
    auto context = sem::StateEvaluationContext<Family, tyr::GroundTag>(state, builder, storage);

    auto column_data = ygg::Data<dl::QueryColumn>();
    column_data.name = "x";
    const auto x = dl::insert(repo, column_data).first.get_index();
    column_data.name = "renamed";
    const auto renamed_column = dl::insert(repo, column_data).first.get_index();
    auto top_data = ygg::Data<dl::Concept<Family, dl::TopTag>>();
    auto top_wrapper = ygg::Data<dl::Constructor<Family, dl::ConceptTag>>(dl::insert(repo, top_data).first.get_index());
    const auto top = dl::insert(repo, top_wrapper).first;
    using Lift = dl::Query<Family, dl::QueryConceptTag>;
    auto lift_data = ygg::Data<Lift>();
    lift_data.arg = top.get_index();
    EXPECT_THROW(static_cast<void>(dl::insert(repo, lift_data)), std::invalid_argument);
    EXPECT_EQ(repo.template size<Lift>(), 0);
    lift_data.columns.push_back(x);
    const auto [lift, created] = dl::insert(repo, lift_data);
    EXPECT_TRUE(created);
    const auto [duplicate_lift, duplicate_created] = dl::insert(repo, lift_data);
    EXPECT_FALSE(duplicate_created);
    EXPECT_EQ(duplicate_lift.get_index(), lift.get_index());
    EXPECT_EQ(repo.template size<Lift>(), 1);
    auto query_data = ygg::Data<dl::Query<Family>>();
    query_data.variant = lift.get_index();
    const auto query = dl::insert(repo, query_data).first;
    auto rename_data = ygg::Data<dl::Query<Family, dl::QueryRenameTag>>();
    rename_data.arg = query.get_index();
    rename_data.columns.push_back(renamed_column);
    query_data.variant = dl::insert(repo, rename_data).first.get_index();
    const auto renamed = dl::insert(repo, query_data).first;

    auto expected_columns = ygg::IndexList<dl::QueryColumn>();
    expected_columns.push_back(renamed_column);
    expected_columns.push_back(x);
    const auto check_inferred_schema = [&](auto& data)
    {
        const auto concrete = dl::insert(repo, data).first;
        EXPECT_EQ(concrete.get_data().columns, expected_columns);

        check_relocated_data(concrete.get_data(),
                             [&](const auto& decoded)
                             {
                                 EXPECT_EQ(decoded.columns, expected_columns);
                                 if constexpr (requires { decoded.schema; })
                                 {
                                     EXPECT_TRUE(std::ranges::equal(ygg::make_view(decoded.schema, repo), concrete.get_schema()));
                                 }
                                 if constexpr (requires { data.plan; })
                                 {
                                     EXPECT_TRUE(std::ranges::equal(decoded.plan.output_columns(), concrete.get_schema()));
                                     EXPECT_TRUE(std::ranges::equal(decoded.plan.lhs_keys(), data.plan.lhs_keys()));
                                     EXPECT_TRUE(std::ranges::equal(decoded.plan.rhs_keys(), data.plan.rhs_keys()));
                                     EXPECT_TRUE(std::ranges::equal(decoded.plan.rhs_payload(), data.plan.rhs_payload()));
                                 }
                                 if constexpr (requires { decoded.lhs_position; })
                                 {
                                     EXPECT_EQ(decoded.lhs_position, 1);
                                     EXPECT_EQ(decoded.rhs_position, 0);
                                 }
                                 if constexpr (requires { decoded.position; })
                                 {
                                     EXPECT_EQ(decoded.position, 1);
                                 }
                             });

        // Derived columns, plans and positions neither change identity nor override child schemas.
        auto stale = data;
        stale.columns.clear();
        stale.columns.push_back(x);
        stale.columns.push_back(x);
        if constexpr (requires { stale.schema; })
            stale.schema = {};
        if constexpr (requires { stale.plan; })
            stale.plan = {};
        if constexpr (requires { stale.lhs_position; })
            stale.lhs_position = stale.rhs_position = 99;
        if constexpr (requires { stale.position; })
            stale.position = 99;
        EXPECT_EQ(stale, concrete.get_data());
        const auto [same, created] = ygg::formalism::insert(repo, stale);
        EXPECT_FALSE(created);
        EXPECT_EQ(same.get_index(), concrete.get_index());
        EXPECT_EQ(stale.columns, expected_columns);
        if constexpr (requires { stale.schema; })
        {
            EXPECT_TRUE(std::ranges::equal(ygg::make_view(stale.schema, repo), concrete.get_schema()));
        }
        if constexpr (requires { stale.plan; })
        {
            EXPECT_TRUE(std::ranges::equal(stale.plan.output_columns(), concrete.get_schema()));
        }
        if constexpr (requires { stale.lhs_position; })
        {
            EXPECT_EQ(stale.lhs_position, 1);
            EXPECT_EQ(stale.rhs_position, 0);
        }
        if constexpr (requires { stale.position; })
        {
            EXPECT_EQ(stale.position, 1);
        }
        stale.clear();
        if constexpr (requires { stale.schema; })
        {
            EXPECT_TRUE(stale.schema.empty());
        }
        if constexpr (requires { stale.plan; })
        {
            EXPECT_TRUE(stale.plan.output_columns().empty());
        }
        if constexpr (requires { stale.lhs_position; })
        {
            EXPECT_EQ(stale.lhs_position, 0);
            EXPECT_EQ(stale.rhs_position, 0);
        }
        if constexpr (requires { stale.position; })
        {
            EXPECT_EQ(stale.position, 0);
        }

        query_data.variant = concrete.get_index();
        const auto wrapper = dl::insert(repo, query_data).first;
        EXPECT_TRUE(std::ranges::equal(wrapper.get_columns(), expected_columns, {}, [](auto column) { return column.get_index(); }));
        return wrapper;
    };
    auto join_data = ygg::Data<dl::Query<Family, dl::QueryJoinTag>>();
    join_data.lhs = renamed.get_index();
    join_data.rhs = query.get_index();
    const auto joined = check_inferred_schema(join_data);
    join_data.lhs = joined.get_index();
    check_inferred_schema(join_data);  // The shared x column is not appended twice.
    check_relocated_data(join_data,
                         [&](const auto& decoded)
                         {
                             ygg::Builder<ygg::database::Relation<>> lhs(decoded.plan.lhs_columns()), rhs(decoded.plan.rhs_columns()),
                                 result(decoded.plan.output_columns());
                             lhs.insert({ 10, 20 });
                             lhs.insert({ 30, 40 });
                             rhs.insert({ 20 });
                             ygg::database::Workspace<> scratch;
                             ygg::database::join(lhs, rhs, decoded.plan, result, scratch);
                             EXPECT_EQ(result.size(), 1);
                             EXPECT_TRUE(result.contains({ 10, 20 }));
                         });
    const auto check_projection_plan = [&](auto& data, auto positions)
    {
        const auto concrete = dl::insert(repo, data).first;
        check_relocated_data(concrete.get_data(),
                             [&](const auto& decoded)
                             {
                                 EXPECT_TRUE(std::ranges::equal(decoded.plan.positions(), positions));
                                 ygg::Builder<ygg::database::Relation<>> input(decoded.plan.input_columns()), result(decoded.plan.output_columns());
                                 std::vector<ygg::uint_t> row(input.arity());
                                 for (size_t i = 0; i < row.size(); ++i)
                                     row[i] = 10 + i;
                                 input.insert(row);
                                 ygg::database::Workspace<> scratch;
                                 ygg::database::project(input, decoded.plan, result, scratch);
                                 ASSERT_EQ(result.size(), 1);
                                 for (size_t i = 0; i < positions.size(); ++i)
                                     EXPECT_EQ(result[0][i], 10 + positions[i]);
                             });
        auto stale = data;
        stale.plan = {};
        EXPECT_EQ(stale, concrete.get_data());
        const auto [same, created] = dl::insert(repo, stale);
        EXPECT_FALSE(created);
        EXPECT_EQ(same.get_index(), concrete.get_index());
        EXPECT_TRUE(std::ranges::equal(stale.plan.positions(), positions));
        stale.clear();
        EXPECT_TRUE(stale.plan.input_columns().empty());
        EXPECT_TRUE(stale.plan.output_columns().empty());
        EXPECT_TRUE(stale.plan.positions().empty());
    };
    auto project_data = ygg::Data<dl::Query<Family, dl::QueryProjectTag>>();
    project_data.arg = joined.get_index();
    project_data.columns.push_back(x);
    project_data.columns.push_back(renamed_column);
    check_projection_plan(project_data, std::array<size_t, 2> { 1, 0 });
    auto role_projection_data = ygg::Data<dl::QueryProjection<Family, dl::RoleTag>>();
    role_projection_data.arg = project_data.arg;
    role_projection_data.columns = project_data.columns;
    check_projection_plan(role_projection_data, std::array<size_t, 2> { 1, 0 });

    auto union_data = ygg::Data<dl::Query<Family, dl::QueryUnionTag>>();
    union_data.lhs = joined.get_index();
    union_data.rhs = joined.get_index();
    check_inferred_schema(union_data);
    auto difference_data = ygg::Data<dl::Query<Family, dl::QueryDifferenceTag>>();
    difference_data.lhs = joined.get_index();
    difference_data.rhs = joined.get_index();
    check_inferred_schema(difference_data);
    auto equal_data = ygg::Data<dl::Query<Family, dl::QuerySelectEqualTag>>();
    equal_data.arg = joined.get_index();
    equal_data.lhs_column = x;
    equal_data.rhs_column = renamed_column;
    check_inferred_schema(equal_data);
    auto value_data = ygg::Data<dl::Query<Family, dl::QuerySelectValueTag>>();
    value_data.arg = joined.get_index();
    value_data.column = x;
    value_data.object = search->task->get_domain().get_domain().get_constants()[0].get_index();
    check_inferred_schema(value_data);

    auto count_data = ygg::Data<dl::Numerical<Family, dl::CountTag>>(renamed.get_index());
    auto count_wrapper = ygg::Data<dl::Constructor<Family, dl::NumericalTag>>(dl::insert(repo, count_data).first.get_index());
    const auto count = dl::insert(repo, count_wrapper).first;
    EXPECT_EQ(sem::evaluate(count, context).get(), 3);
    EXPECT_EQ(sem::evaluate(count, context).get(), 3);
    EXPECT_TRUE(caches.template get<dl::ConceptTag>(true).contains(top));

    auto projection_data = ygg::Data<dl::QueryProjection<Family, dl::ConceptTag>>();
    projection_data.arg = renamed.get_index();
    projection_data.columns.push_back(renamed_column);
    check_projection_plan(projection_data, std::array<size_t, 1> { 0 });
    auto projection_wrapper = ygg::Data<dl::Constructor<Family, dl::ConceptTag>>(dl::insert(repo, projection_data).first.get_index());
    const auto projection = dl::insert(repo, projection_wrapper).first;
    EXPECT_EQ(sem::evaluate(projection, context).get().count(), 3);

    const auto held = sem::evaluate(renamed, context);
    const auto* held_storage = held.get_storage_address();
    const auto* held_columns = held.columns().data();
    EXPECT_EQ(held.columns()[0], ColumnIndex(ygg::uint_t(renamed_column)));
    EXPECT_NE(held_columns, renamed.get_schema().data());
    EXPECT_TRUE(std::ranges::equal(held.columns(), renamed.get_schema()));
    EXPECT_EQ(sem::evaluate(query, context).get_storage_address(), held_storage);
    auto nested_rename_data = rename_data;
    nested_rename_data.arg = renamed.get_index();
    nested_rename_data.columns[0] = x;
    query_data.variant = dl::insert(repo, nested_rename_data).first.get_index();
    const auto nested = sem::evaluate(dl::insert(repo, query_data).first, context);
    EXPECT_EQ(nested.columns()[0], ColumnIndex(ygg::uint_t(x)));
    EXPECT_EQ(nested.get_storage_address(), held_storage);
    EXPECT_EQ(nested.size(), held.size());
    EXPECT_EQ(held.columns()[0], ColumnIndex(ygg::uint_t(renamed_column)));
    // Owned labels and shared rows remain valid during repository/cache growth.
    for (int i = 0; i < 64; ++i)
    {
        column_data.name = "extra_" + std::to_string(i);
        const auto extra_column = dl::insert(repo, column_data).first.get_index();
        rename_data.columns[0] = extra_column;
        query_data.variant = dl::insert(repo, rename_data).first.get_index();
        const auto extra_query = dl::insert(repo, query_data).first;
        const auto other = sem::evaluate(extra_query, context);
        EXPECT_EQ(other.get_storage_address(), held_storage);
        EXPECT_EQ(held.size(), 3);
        EXPECT_EQ(held.columns().data(), held_columns);
        EXPECT_EQ(held.columns()[0], ColumnIndex(ygg::uint_t(renamed_column)));
        EXPECT_EQ(other.columns()[0], ColumnIndex(ygg::uint_t(extra_column)));
    }

    // Repositories from one factory have distinct identities even when query indices match.
    auto& other_repo = *other_repository;
    EXPECT_NE(other_repo.get_index(), repo.get_index());
    column_data.name = "unused";
    static_cast<void>(dl::insert(other_repo, column_data));
    column_data.name = "other";
    const auto other_column = dl::insert(other_repo, column_data).first.get_index();
    top_wrapper.variant = dl::insert(other_repo, top_data).first.get_index();
    lift_data.arg = dl::insert(other_repo, top_wrapper).first.get_index();
    lift_data.columns[0] = other_column;
    query_data.variant = dl::insert(other_repo, lift_data).first.get_index();
    const auto other_query = dl::insert(other_repo, query_data).first;
    ASSERT_EQ(other_query.get_index(), query.get_index());
    ASSERT_NE(other_column, x);
    const auto cached = sem::evaluate(query, context);
    const auto other_cached = sem::evaluate(other_query, context);
    EXPECT_EQ(cached.columns()[0], ColumnIndex(ygg::uint_t(x)));
    EXPECT_EQ(other_cached.columns()[0], ColumnIndex(ygg::uint_t(other_column)));
    EXPECT_EQ(cached.size(), 3);
    EXPECT_EQ(other_cached.size(), 3);
    EXPECT_NE(cached, other_cached);
    EXPECT_EQ(cached.get_storage_address(), other_cached.get_storage_address());
    EXPECT_EQ(sem::evaluate(query, context).get_storage_address(), cached.get_storage_address());
    EXPECT_EQ(sem::evaluate(other_query, context).get_storage_address(), other_cached.get_storage_address());

    // Clear invalidates the borrowed views; reevaluation reconstructs rows and schemas.
    storage.reset_all();
    EXPECT_TRUE(caches.get_queries(true).empty());
    EXPECT_TRUE(caches.get_queries(false).empty());
    const auto rebuilt = sem::evaluate(renamed, context);
    EXPECT_EQ(rebuilt.size(), 3);
    EXPECT_EQ(rebuilt.columns()[0], ColumnIndex(ygg::uint_t(renamed_column)));
    EXPECT_EQ(rebuilt.get_storage_address(), sem::evaluate(query, context).get_storage_address());
    storage.reset_all();
    repo.clear();
}

template<tyr::TaskKind Kind>
void check_query_cache_across_states()
{
    const auto search = query_search_context<Kind>();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = sem::Builder();
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<Ext>(denotations);
    auto& caches = storage.get_caches();
    auto empty_arguments = ygg::Data<sem::CallArguments>();
    auto registers = ygg::Data<sem::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = sem::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                          builder,
                                                          storage,
                                                          sem::insert(denotations, empty_arguments).first,
                                                          sem::insert(denotations, registers).first);
    const auto fixed = parse_query(R"((q_atomic_state "fixed" (x y z)))", domain, *repository);
    const auto goal = parse_query(R"((q_atomic_goal "triple" false (x y z)))", domain, *repository);
    const auto derived = parse_query(R"((q_atomic_state "copied" (x y z)))", domain, *repository);
    const auto ready = parse_query(R"((q_atomic_state "ready" ()))", domain, *repository);
    const auto triple = parse_query(R"((q_atomic_state "triple" (x y z)))", domain, *repository);
    const auto fixed_join = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_atomic_state "triple" (x y z))))", domain, *repository);
    const auto reversed_join = parse_query(R"((q_join (q_atomic_state "triple" (x y z)) (q_atomic_state "fixed" (x y z))))", domain, *repository);
    const auto static_join = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_atomic_state "fixed" (x y z))))", domain, *repository);
    const auto count = parser::parse_numerical(R"((n_count (q_atomic_state "triple" (x y z))))", domain, *repository);
    const auto fixed_count = parser::parse_numerical(R"((n_count (q_atomic_state "fixed" (x y z))))", domain, *repository);
    const auto matching_row =
        std::array { domain.get_constants()[0].get_index(), domain.get_constants()[1].get_index(), domain.get_constants()[2].get_index() };
    const auto check_mixed_joins = [&](auto& target)
    {
        const auto matches = sem::evaluate(triple, target).contains(matching_row);
        for (const auto query : { fixed_join, reversed_join })
        {
            const auto rows = sem::evaluate(query, target);
            EXPECT_EQ(rows.size(), matches ? 1 : 0);
            EXPECT_EQ(rows.contains(matching_row), matches);
        }
        return matches;
    };

    const auto fixed_rows = sem::evaluate(fixed, context);
    const auto goal_rows = sem::evaluate(goal, context);
    EXPECT_EQ(fixed_rows.size(), 2);
    EXPECT_EQ(goal_rows.size(), 1);
    EXPECT_EQ(sem::evaluate(fixed, context).get_storage_address(), fixed_rows.get_storage_address());
    EXPECT_EQ(sem::evaluate(derived, context).size(), 4);
    EXPECT_EQ(sem::evaluate(ready, context).size(), 1);
    EXPECT_EQ(sem::evaluate(count, context).get(), 4);
    EXPECT_EQ(sem::evaluate(fixed_count, context).get(), 2);
    // A fully static join caches its result without retaining an additional index.
    EXPECT_EQ(sem::evaluate(static_join, context).size(), fixed_rows.size());
    EXPECT_EQ(caches.get_static_join_indexes().size(), 0);
    EXPECT_TRUE(check_mixed_joins(context));
    // Opposite join orientations share one index over the same static rows and keys.
    EXPECT_EQ(caches.get_static_join_indexes().size(), 1);

    const auto successors = search->successor_generator->get_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator);
    ASSERT_FALSE(successors.empty());
    // Only removing (triple a b c) empties the join; the static index survives state changes.
    auto empty_joins = size_t { 0 };
    for (const auto& successor : successors)
    {
        auto successor_context = sem::StateEvaluationContext<Ext, Kind>(successor.get_state(), builder, storage, context.arguments(), context.registers());
        storage.reset_dynamic();
        empty_joins += !check_mixed_joins(successor_context);
        EXPECT_EQ(caches.get_static_join_indexes().size(), 1);
    }
    EXPECT_EQ(empty_joins, 1);
    auto next = sem::StateEvaluationContext<Ext, Kind>(successors.front().get_state(),
                                                       builder,
                                                       storage,
                                                       sem::insert(denotations, empty_arguments).first,
                                                       sem::insert(denotations, registers).first);
    storage.reset_dynamic();
    EXPECT_TRUE(caches.get_queries(false).empty());
    EXPECT_TRUE(caches.template get<dl::NumericalTag>(false).empty());
    EXPECT_TRUE(caches.get_queries(true).contains(fixed));
    EXPECT_TRUE(caches.template get<dl::NumericalTag>(true).contains(fixed_count));
    EXPECT_EQ(sem::evaluate(fixed, next).get_storage_address(), fixed_rows.get_storage_address());
    EXPECT_EQ(sem::evaluate(goal, next).get_storage_address(), goal_rows.get_storage_address());
    EXPECT_EQ(sem::evaluate(derived, next).size(), 3);
    EXPECT_TRUE(sem::evaluate(ready, next).empty());
    EXPECT_EQ(sem::evaluate(count, next).get(), 3);

    const auto dynamic_role = parser::parse_role(R"((r_project x z (q_atomic_state "triple" (x y z))))", domain, *repository);
    const auto static_role = parser::parse_role(R"((r_atomic_state "edge"))", domain, *repository);
    const auto static_value = sem::evaluate(static_role, context);
    for (size_t i = 0; i < 16; ++i)
    {
        storage.reset_dynamic();
        EXPECT_EQ(storage.get_denotation_repository(false).template size<sem::Denotation<dl::RoleTag>>(), 0);
        EXPECT_EQ(static_value.count(), 2);
        const auto value = sem::evaluate(dynamic_role, i % 2 == 0 ? context : next);
        EXPECT_EQ(value.count(), i % 2 == 0 ? 4 : 3);
        EXPECT_EQ(storage.get_denotation_repository(false).template size<sem::Denotation<dl::RoleTag>>(), 1);
        EXPECT_EQ(denotations.template size<sem::Denotation<dl::RoleTag>>(), 0);
    }

    // Materialize persistent results directly; source and target scratch storage remain independent.
    storage.reset_dynamic();
    auto persistent_memo = sem::DenotationCaches<Ext> {};
    auto persistent = persistent_context(context, persistent_memo, denotations, storage);
    const auto retained = sem::evaluate(dynamic_role, persistent);
    EXPECT_EQ(storage.get_denotation_repository(false).template size<sem::Denotation<dl::RoleTag>>(), 0);
    EXPECT_FALSE(caches.template get<dl::RoleTag>(false).contains(dynamic_role));
    const auto source_value = sem::evaluate(dynamic_role, context);
    auto target_storage = sem::EvaluationStorage<Ext>(denotations);
    EXPECT_NE(&target_storage.get_denotation_repository(false), &storage.get_denotation_repository(false));
    EXPECT_NE(&storage.get_denotation_repository(false), &storage.get_denotation_repository(true));
    auto target_context = sem::StateEvaluationContext<Ext, Kind>(successors.front().get_state(), builder, target_storage, next.arguments(), next.registers());
    EXPECT_EQ(sem::evaluate(dynamic_role, target_context).count(), 3);
    EXPECT_NE(sem::evaluate(static_role, target_context), static_value);
    target_storage.reset_all();
    EXPECT_EQ(source_value.count(), 4);
    EXPECT_EQ(static_value.count(), 2);
    EXPECT_EQ(retained.count(), 4);

    storage.reset_all();
    EXPECT_EQ(caches.get_static_join_indexes().size(), 0);
    EXPECT_TRUE(caches.get_queries(true).empty());
    EXPECT_TRUE(caches.template get<dl::NumericalTag>(true).empty());
    EXPECT_TRUE(caches.get_queries(false).empty());
    EXPECT_TRUE(caches.template get<dl::NumericalTag>(false).empty());
    EXPECT_EQ(storage.get_denotation_repository(false).template size<sem::Denotation<dl::RoleTag>>(), 0);
    EXPECT_EQ(storage.get_denotation_repository(true).template size<sem::Denotation<dl::RoleTag>>(), 0);
    EXPECT_EQ(retained.count(), 4);
    // A full clear releases indexed rows as well; reevaluation must rebuild the shared index.
    EXPECT_TRUE(check_mixed_joins(context));
    EXPECT_EQ(caches.get_static_join_indexes().size(), 1);
    storage.reset_all();
    EXPECT_TRUE(caches.get_queries(false).empty());
    EXPECT_TRUE(caches.template get<dl::NumericalTag>(false).empty());
}

template<tyr::TaskKind Kind>
void check_query_cache_across_bindings()
{
    const auto search = query_search_context<Kind>();
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = sem::Builder();
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<Ext>(denotations);
    auto& caches = storage.get_caches();
    auto empty_arguments = ygg::Data<sem::CallArguments>();
    auto registers = ygg::Data<sem::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = sem::StateEvaluationContext<Ext, Kind>(state,
                                                          builder,
                                                          storage,
                                                          sem::insert(denotations, empty_arguments).first,
                                                          sem::insert(denotations, registers).first);
    const auto a = domain.get_constants()[0];
    const auto b = domain.get_constants()[1];
    const auto c = domain.get_constants()[2];
    auto persistent_memo = sem::DenotationCaches<Ext> {};
    auto persistent = persistent_context(context, persistent_memo, denotations, storage);
    const auto a_set = sem::evaluate(parser::parse_concept(R"((c_nominal "a"))", domain, *repository), persistent);
    const auto b_set = sem::evaluate(parser::parse_concept(R"((c_nominal "b"))", domain, *repository), persistent);
    auto argument_values = ygg::Data<sem::CallArguments>();
    argument_values.concept_arguments.push_back(a_set.get_index());
    const auto make_bound = [&]
    {
        return sem::StateEvaluationContext<Ext, Kind>(state,
                                                      builder,
                                                      storage,
                                                      sem::insert(denotations, argument_values).first,
                                                      sem::insert(denotations, registers).first);
    };
    registers.concept_values[0] = a.get_index();
    registers.role_values[0] = ::cista::pair(a.get_index(), b.get_index());
    auto bound = make_bound();
    const auto fixed = parse_query(R"((q_atomic_state "fixed" (x y z)))", domain, *repository);
    const auto register_count =
        parser::parse_numerical(R"((n_count (q_join (q_atomic_state "fixed" (x y z)) (q_concept x (c_register 0)))))", domain, *repository);
    const auto argument = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_concept x (c_argument 0))))", domain, *repository);
    const auto role_register = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_role (x y) (r_register 0))))", domain, *repository);
    const auto register_rhs = parse_query(R"((q_join (q_concept z (c_register 0)) (q_atomic_state "fixed" (x y z))))", domain, *repository);
    const auto role_register_rhs = parse_query(R"((q_join (q_role (x y) (r_register 0)) (q_atomic_state "fixed" (y x z))))", domain, *repository);

    const auto fixed_rows = sem::evaluate(fixed, bound);
    EXPECT_EQ(sem::evaluate(register_count, bound).get(), 1);
    EXPECT_EQ(sem::evaluate(argument, bound).size(), 1);
    EXPECT_EQ(sem::evaluate(role_register, bound).size(), 1);
    const auto rhs_rows = sem::evaluate(register_rhs, bound);
    EXPECT_EQ(rhs_rows.size(), 1);
    EXPECT_TRUE(rhs_rows.contains({ a.get_index(), c.get_index(), b.get_index() }));
    EXPECT_TRUE(std::ranges::equal(rhs_rows.columns(), register_rhs.get_schema()));
    EXPECT_TRUE(sem::evaluate(role_register_rhs, bound).empty());
    registers.concept_values[0] = b.get_index();
    registers.role_values[0] = ::cista::pair(b.get_index(), c.get_index());
    argument_values.concept_arguments[0] = b_set.get_index();
    auto rebound = make_bound();
    storage.reset_dynamic();
    EXPECT_EQ(sem::evaluate(fixed, rebound).get_storage_address(), fixed_rows.get_storage_address());
    EXPECT_EQ(sem::evaluate(register_count, rebound).get(), 0);
    EXPECT_TRUE(sem::evaluate(argument, rebound).empty());
    EXPECT_TRUE(sem::evaluate(role_register, rebound).empty());
    EXPECT_TRUE(sem::evaluate(register_rhs, rebound).empty());
    // Reordered shared columns are matched by label, not by their position in either operand.
    const auto rhs_role_rows = sem::evaluate(role_register_rhs, rebound);
    EXPECT_EQ(rhs_role_rows.size(), 1);
    EXPECT_TRUE(rhs_role_rows.contains({ b.get_index(), c.get_index(), a.get_index() }));
    EXPECT_TRUE(std::ranges::equal(rhs_role_rows.columns(), role_register_rhs.get_schema()));
    EXPECT_TRUE(caches.template get<dl::NumericalTag>(false).contains(register_count));
    EXPECT_FALSE(caches.template get<dl::NumericalTag>(true).contains(register_count));

    registers.concept_values[0].reset();
    registers.role_values[0].reset();
    argument_values.concept_arguments[0] = a_set.get_index();
    auto cleared = make_bound();
    storage.reset_dynamic();
    EXPECT_EQ(sem::evaluate(register_count, cleared).get(), 0);
    EXPECT_EQ(sem::evaluate(argument, cleared).size(), 1);
    EXPECT_TRUE(sem::evaluate(role_register, cleared).empty());
    EXPECT_TRUE(sem::evaluate(register_rhs, cleared).empty());
    EXPECT_TRUE(sem::evaluate(role_register_rhs, cleared).empty());
    EXPECT_EQ(bound.registers().template get<dl::ConceptTag>()[0].value().get_index(), a.get_index());
    EXPECT_EQ(bound.arguments().template get<dl::ConceptTag>()[0].get_index(), a_set.get_index());
    EXPECT_EQ(rebound.registers().template get<dl::ConceptTag>()[0].value().get_index(), b.get_index());
    EXPECT_EQ(rebound.arguments().template get<dl::ConceptTag>()[0].get_index(), b_set.get_index());
}

template<tyr::TaskKind Kind>
void check_predicate_repository_identity()
{
    using Family = kr::BaseFamilyTag;
    using Fact = tyr::formalism::FluentTag;
    using Predicate = tyr::formalism::Predicate<Fact>;
    namespace fp = tyr::formalism::planning;
    const auto domain_source = std::string(R"(
(define (domain predicate-identity)
  (:requirements :strips)
  (:constants a b)
  (:predicates (flag) (marked ?x) (edge ?x ?y))
  (:action clear :parameters (?x ?y)
    :precondition (and (flag) (marked ?x) (edge ?x ?y))
    :effect (and (not (flag)) (not (marked ?x)) (not (edge ?x ?y)))))
)");
    const auto task_source = std::string(R"(
(define (problem predicate-identity-task)
  (:domain predicate-identity)
  (:init (flag) (marked a) (edge a b))
  (:goal (and (flag) (marked a) (edge a b))))
)");
    auto execution = ygg::ExecutionContext::create(1);
    auto lifted = tyr::planning::Task<tyr::LiftedTag>::create(fp::Parser(domain_source, std::nullopt).parse_task(task_source, std::nullopt));
    auto task = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return lifted->instantiate_ground_task(*execution).task;
        else
            return lifted;
    }();
    auto search = datasets::TaskSearchContext<Kind>::create(task, execution);
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto& domain = task->get_domain();
    // Use one factory so repository identities differ even though predicate indices collide.
    const auto foreign = domain.get_repository_factory()->create_shared();
    auto predicates = std::array<ygg::Index<Predicate>, 3> {};
    ASSERT_EQ(domain.get_domain().template get_predicates<Fact>().size(), predicates.size());
    for (const auto predicate : domain.get_domain().template get_predicates<Fact>())
    {
        ASSERT_LT(predicate.get_arity(), predicates.size());
        predicates[predicate.get_arity()] = predicate.get_index();
        auto data = ygg::Data<Predicate>("foreign_" + predicate.get_name().str(), predicate.get_arity());
        const auto other = fp::insert(*foreign, data).first;
        ASSERT_EQ(other.get_index(), predicate.get_index());
        ASSERT_NE(other, predicate);
        EXPECT_EQ(ygg::make_view(predicate.get_index(), *task->get_repository()), predicate);
    }
    auto constructor_factory = dl::ConstructorRepositoryFactoryFor<Family>();
    for (const auto& planning_repository : { domain.get_repository(), task->get_repository(), foreign })
    {
        const auto expected = planning_repository != foreign;
        SCOPED_TRACE(expected ? "same predicate owner" : "different predicate owner");
        auto repository = constructor_factory.create(planning_repository);
        auto builder = sem::Builder();
        auto denotations = sem::DenotationRepositoryFactory().create(task->get_repository());
        auto storage = sem::EvaluationStorage<Family>(denotations);
        auto context = sem::StateEvaluationContext<Family, Kind>(initial.get_state(), builder, storage);
        const auto check = [&]<typename Tag>()
        {
            auto concept_data = ygg::Data<dl::Concept<Family, Tag>>(predicates[1]);
            auto concept_wrapper = ygg::Data<dl::Constructor<Family, dl::ConceptTag>>(dl::insert(*repository, concept_data).first.get_index());
            EXPECT_EQ(sem::evaluate(dl::insert(*repository, concept_wrapper).first, context).get().count(), size_t(expected));

            auto role_data = ygg::Data<dl::Role<Family, Tag>>(predicates[2]);
            auto role_wrapper = ygg::Data<dl::Constructor<Family, dl::RoleTag>>(dl::insert(*repository, role_data).first.get_index());
            EXPECT_EQ(sem::evaluate(dl::insert(*repository, role_wrapper).first, context).count(), size_t(expected));

            auto boolean_data = ygg::Data<dl::Boolean<Family, Tag>>(predicates[0]);
            auto boolean_wrapper = ygg::Data<dl::Constructor<Family, dl::BooleanTag>>(dl::insert(*repository, boolean_data).first.get_index());
            EXPECT_EQ(sem::evaluate(dl::insert(*repository, boolean_wrapper).first, context).get(), expected);

            auto column = ygg::Data<dl::QueryColumn>();
            column.name = "x";
            auto query_data = ygg::Data<dl::Query<Family, Tag>>();
            query_data.predicate = predicates[1];
            query_data.columns.push_back(dl::insert(*repository, column).first.get_index());
            if constexpr (dl::is_atomic_goal_tag_v<Tag>)
                query_data.polarity = true;
            auto query_wrapper = ygg::Data<dl::Query<Family>>();
            query_wrapper.variant = dl::insert(*repository, query_data).first.get_index();
            EXPECT_EQ(sem::evaluate(dl::insert(*repository, query_wrapper).first, context).size(), size_t(expected));
        };
        check.template operator()<dl::AtomicStateTag<Fact>>();
        check.template operator()<dl::AtomicGoalTag<Fact>>();
    }
}

template<tyr::TaskKind Kind>
void check_query_object_views()
{
    namespace fp = tyr::formalism::planning;
    const auto execution = ygg::ExecutionContext::create(1);
    const auto lifted = tyr::planning::Task<tyr::LiftedTag>::create(fp::Parser(R"(
(define (domain query-objects)
  (:requirements :strips)
  (:constants parent)
  (:predicates (fixed ?x ?y)))
)",
                                                                               "query-objects-domain.pddl")
                                                                        .parse_task(R"(
(define (problem query-objects-task)
  (:domain query-objects)
  (:objects local)
  (:init (fixed parent local))
  (:goal (fixed parent local)))
)",
                                                                                    "query-objects-task.pddl"));
    const auto task = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return lifted->instantiate_ground_task(*execution).task;
        else
            return lifted;
    }();
    ASSERT_TRUE(task);
    const auto search = datasets::TaskSearchContext<Kind>::create(task, execution);
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    auto constructors = dl::ConstructorRepositoryFactoryFor<Ext>().create(task->get_repository());
    auto denotations = sem::DenotationRepositoryFactory().create(task->get_repository());
    auto builder = sem::Builder {};
    auto storage = sem::EvaluationStorage<Ext>(denotations);
    auto arguments = ygg::Data<sem::CallArguments> {};
    auto registers = ygg::Data<sem::RegisterValues> {};
    auto context =
        sem::StateEvaluationContext<Ext, Kind>(state, builder, storage, sem::insert(denotations, arguments).first, sem::insert(denotations, registers).first);
    const auto query = parse_query(R"((q_atomic_state "fixed" (x y)))", task->get_domain().get_domain(), *constructors);
    const auto constant = task->get_task().get_domain().get_constants()[0];
    const auto local = task->get_task().get_objects()[0];
    ASSERT_NE(&constant.get_context(), &local.get_context());
    const auto expected = std::array { constant, local };
    const auto raw = std::array { constant.get_index(), local.get_index() };
    const auto result = sem::evaluate(query, context);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(&result.get_context(), &storage.get_denotation_repository(true));
    const auto row = result[0];
    static_assert(std::same_as<std::remove_cvref_t<decltype(row)>, fp::ObjectSpanView>);
    EXPECT_EQ(&row.get_context(), task->get_repository().get());
    EXPECT_TRUE(std::ranges::equal(result.row(0), raw));
    EXPECT_TRUE(result.contains(raw));
    EXPECT_TRUE(std::ranges::equal(row, expected));
    EXPECT_EQ(row[0].get_name(), "parent");
    EXPECT_EQ(row[1].get_name(), "local");
    EXPECT_EQ(&row[0].get_context(), &constant.get_context());
    EXPECT_EQ(&row[1].get_context(), &local.get_context());

    storage.reset_dynamic();
    EXPECT_TRUE(std::ranges::equal(row, expected));
    EXPECT_EQ(sem::evaluate(query, context), result);
    storage.reset_all();
    const auto rebuilt = sem::evaluate(query, context);
    ASSERT_EQ(rebuilt.size(), 1);
    EXPECT_TRUE(std::ranges::equal(rebuilt[0], expected));
}

template<tyr::TaskKind Kind>
void check_multiword_bit_evaluation()
{
    constexpr auto word_bits = ygg::uint_t(std::numeric_limits<ygg::uint_t>::digits);
    constexpr auto num_objects = 2 * word_bits + 3;
    constexpr auto last = num_objects - 1;
    const auto name = [](ygg::uint_t i) { return "v" + std::to_string(1000 + i); };
    auto domain_source = std::string("(define (domain bit-boundaries) (:requirements :strips) (:constants");
    for (ygg::uint_t i = 0; i < num_objects; ++i)
        domain_source += " " + name(i);
    domain_source += ") (:predicates (marked ?x) (edge ?x ?y)))";
    auto task_source = std::string("(define (problem bit-boundaries-task) (:domain bit-boundaries) (:init");
    for (const auto i : { ygg::uint_t(0), word_bits - 1, word_bits, last })
        task_source += " (marked " + name(i) + ")";
    for (const auto& [from, to] : std::array { std::pair { ygg::uint_t(0), word_bits - 1 },
                                               std::pair { ygg::uint_t(0), word_bits },
                                               std::pair { word_bits - 1, last },
                                               std::pair { word_bits, last } })
        task_source += " (edge " + name(from) + " " + name(to) + ")";
    task_source += ") (:goal (and)))";
    auto execution = ygg::ExecutionContext::create(1);
    auto lifted =
        tyr::planning::Task<tyr::LiftedTag>::create(tyr::formalism::planning::Parser(domain_source, std::nullopt).parse_task(task_source, std::nullopt));
    auto task = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return lifted->instantiate_ground_task(*execution).task;
        else
            return lifted;
    }();
    auto search = datasets::TaskSearchContext<Kind>::create(task, execution);
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = task->get_domain().get_domain();
    ASSERT_EQ(domain.get_constants().size(), num_objects);
    for (const auto i : { ygg::uint_t(0), word_bits - 1, word_bits, last })
    {
        ASSERT_EQ(domain.get_constants()[i].get_name(), name(i));
        ASSERT_EQ(ygg::uint_t(domain.get_constants()[i].get_index()), i);
    }
    auto repository = dl::ConstructorRepositoryFactoryFor<Ext>().create(task->get_repository());
    sem::Builder builder;
    auto denotations = sem::DenotationRepositoryFactory().create(task->get_repository());
    sem::EvaluationStorage<Ext> storage(denotations);
    ygg::Data<sem::CallArguments> arguments;
    ygg::Data<sem::RegisterValues> registers;
    auto context = sem::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                          builder,
                                                          storage,
                                                          sem::insert(denotations, arguments).first,
                                                          sem::insert(denotations, registers).first);
    const auto query = [&](const std::string& expression) { return sem::evaluate(parse_query(expression, domain, *repository), context); };
    const auto concept_value = [&](const std::string& expression) { return sem::evaluate(parser::parse_concept(expression, domain, *repository), context); };
    const auto number = [&](const std::string& expression) { return sem::evaluate(parser::parse_numerical(expression, domain, *repository), context).get(); };
    const auto marked = std::string(R"((c_atomic_state "marked"))");
    const auto edge = std::string(R"((r_atomic_state "edge"))");

    const auto selected = query("(q_concept x " + marked + ")");
    ASSERT_EQ(selected.size(), 4);
    for (const auto i : { ygg::uint_t(0), word_bits - 1, word_bits, last })
        EXPECT_TRUE(selected.contains({ ObjectIndex(i) }));
    EXPECT_EQ(query("(q_concept x (c_top))").size(), num_objects);
    EXPECT_TRUE(query("(q_concept x (c_bot))").empty());
    const auto identity = query("(q_role (x y) (r_identity (c_top)))");
    EXPECT_EQ(identity.size(), num_objects);
    EXPECT_TRUE(identity.contains({ ObjectIndex(last), ObjectIndex(last) }));
    EXPECT_TRUE(query("(q_role (x y) (r_identity (c_bot)))").empty());

    const auto inverse = query("(q_role (x y) (r_inverse " + edge + "))");
    ASSERT_EQ(inverse.size(), 4);
    EXPECT_TRUE(inverse.contains({ ObjectIndex(word_bits - 1), ObjectIndex(0) }));
    EXPECT_TRUE(inverse.contains({ ObjectIndex(word_bits), ObjectIndex(0) }));
    EXPECT_TRUE(inverse.contains({ ObjectIndex(last), ObjectIndex(word_bits - 1) }));
    EXPECT_TRUE(inverse.contains({ ObjectIndex(last), ObjectIndex(word_bits) }));
    const auto composition = query("(q_role (x y) (r_composition " + edge + " " + edge + "))");
    ASSERT_EQ(composition.size(), 1);
    EXPECT_TRUE(composition.contains({ ObjectIndex(0), ObjectIndex(last) }));
    EXPECT_TRUE(query("(q_role (x y) (r_composition " + edge + " (r_identity (c_bot))))").empty());

    const auto at_least = concept_value("(c_at_least 2 " + edge + " " + marked + ")").get();
    EXPECT_EQ(at_least.count(), 1);
    EXPECT_TRUE(at_least.test(0));
    const auto exactly = concept_value("(c_exactly 1 " + edge + " " + marked + ")").get();
    EXPECT_EQ(exactly.count(), 2);
    EXPECT_TRUE(exactly.test(word_bits - 1));
    EXPECT_TRUE(exactly.test(word_bits));
    const auto at_most = concept_value("(c_at_most 1 " + edge + " " + marked + ")").get();
    EXPECT_EQ(at_most.count(), num_objects - 1);
    EXPECT_FALSE(at_most.test(0));
    EXPECT_EQ(concept_value("(c_exactly 0 " + edge + " (c_bot))").get().count(), num_objects);

    // The second source crosses a block boundary and shortens the path from two steps to one.
    const auto starts = "(c_or (c_nominal \"" + name(0) + "\") (c_nominal \"" + name(word_bits) + "\"))";
    EXPECT_EQ(number("(n_distance " + starts + " " + edge + " (c_nominal \"" + name(last) + "\"))"), 1);
    EXPECT_EQ(number("(n_distance (c_nominal \"" + name(last) + "\") " + edge + " (c_nominal \"" + name(0) + "\"))"), std::numeric_limits<ygg::uint_t>::max());
}

}  // namespace

TEST(RunirQueries, GroundQueryRowsResolveCanonicalObjectViews) { check_query_object_views<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedQueryRowsResolveCanonicalObjectViews) { check_query_object_views<tyr::LiftedTag>(); }
TEST(RunirQueries, GroundMultiwordBitEvaluation) { check_multiword_bit_evaluation<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedMultiwordBitEvaluation) { check_multiword_bit_evaluation<tyr::LiftedTag>(); }
TEST(RunirQueries, GroundAtomicEvaluationChecksPredicateRepository) { check_predicate_repository_identity<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedAtomicEvaluationChecksPredicateRepository) { check_predicate_repository_identity<tyr::LiftedTag>(); }
TEST(RunirQueries, GroundRelationsAndMutableBindings) { check_queries<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedRelationsAndMutableBindings) { check_queries<tyr::LiftedTag>(); }
TEST(RunirQueries, BaseCachingAndBorrowedRenameLifetime) { check_cached_queries<kr::BaseFamilyTag>(); }
TEST(RunirQueries, UnsInheritsQueryEvaluation) { check_cached_queries<kr::UnsFamilyTag>(); }
TEST(RunirQueries, GroundQueryCachePreservesStaticResultsAcrossStates) { check_query_cache_across_states<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedQueryCachePreservesStaticResultsAcrossStates) { check_query_cache_across_states<tyr::LiftedTag>(); }
TEST(RunirQueries, GroundQueryCacheInvalidatesRegistersAndArguments) { check_query_cache_across_bindings<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedQueryCacheInvalidatesRegistersAndArguments) { check_query_cache_across_bindings<tyr::LiftedTag>(); }

TEST(RunirQueries, StaticnessIsInferredAndSerializedWithoutChangingIdentity)
{
    const auto search = query_search_context<tyr::GroundTag>();
    const auto domain = search->task->get_domain().get_domain();
    auto repository = dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    const auto fixed = std::string(R"((q_atomic_state "fixed" (x y z)))");
    const auto triple = std::string(R"((q_atomic_state "triple" (x y z)))");
    const auto cases = std::vector<std::pair<std::string, bool>> {
        { fixed, true },
        { triple, false },
        { R"((q_atomic_state "copied" (x y z)))", false },
        { R"((q_atomic_goal "triple" true (x y z)))", true },
        { R"((q_atomic_goal "triple" false (x y z)))", true },
        { "(q_join " + fixed + " " + fixed + ")", true },
        { "(q_join " + fixed + " " + triple + ")", false },
        { "(q_project () " + fixed + ")", true },
        { "(q_rename (a b c) " + triple + ")", false },
        { "(q_select_equal y z " + fixed + ")", true },
        { "(q_select_value x \"a\" " + triple + ")", false },
        { "(q_union " + fixed + " " + fixed + ")", true },
        { "(q_difference " + fixed + " " + triple + ")", false },
        { R"((q_concept x (c_top)))", true },
        { R"((q_concept x (c_register 0)))", false },
        { R"((q_concept x (c_argument 0)))", false },
        { R"((q_role (x y) (r_atomic_state "edge")))", true },
        { R"((q_role (x y) (r_register 0)))", false },
        { R"((q_role (x y) (r_argument 0)))", false },
    };
    const auto check_metadata = [&](auto expression, bool expected)
    {
        EXPECT_EQ(expression.is_static(), expected);
        check_relocated_data(expression.get_data(), [&](const auto& decoded) { EXPECT_EQ(decoded.is_static, expected); });
        auto stale = expression.get_data();
        stale.is_static = !expected;
        EXPECT_EQ(stale, expression.get_data());
        using Data = std::remove_cvref_t<decltype(stale)>;
        EXPECT_EQ(ygg::Hash<Data>()(stale), ygg::Hash<Data>()(expression.get_data()));
        const auto [same, created] = ygg::formalism::insert(*repository, stale);
        EXPECT_FALSE(created);
        EXPECT_EQ(same.get_index(), expression.get_index());
        EXPECT_EQ(stale.is_static, expected);
        stale.clear();
        EXPECT_FALSE(stale.is_static);
    };
    for (const auto& [description, expected] : cases)
    {
        SCOPED_TRACE(description);
        check_metadata(parse_query(description, domain, *repository), expected);
        check_metadata(parser::parse_numerical("(n_count " + description + ")", domain, *repository), expected);
    }
    EXPECT_TRUE(parser::parse_concept("(c_project x " + fixed + ")", domain, *repository).is_static());
    EXPECT_FALSE(parser::parse_concept("(c_project x " + triple + ")", domain, *repository).is_static());
    EXPECT_TRUE(parser::parse_role(R"((r_reflexive_transitive_closure (r_atomic_state "edge")))", domain, *repository).is_static());
    EXPECT_FALSE(parser::parse_role(R"((r_reflexive_transitive_closure (r_register 0)))", domain, *repository).is_static());
    EXPECT_TRUE(parser::parse_boolean("(b_nonempty " + fixed + ")", domain, *repository).is_static());
    EXPECT_FALSE(parser::parse_boolean("(b_nonempty " + triple + ")", domain, *repository).is_static());
}

TEST(RunirQueries, CachedRenameSharesRowsWithoutChangingSchemasAndHandlesNullaryJoins)
{
    const auto search = query_search_context<tyr::GroundTag>();
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = sem::Builder();
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<Ext>(denotations);
    auto& caches = storage.get_caches();
    auto empty_arguments = ygg::Data<sem::CallArguments>();
    auto registers = ygg::Data<sem::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = sem::StateEvaluationContext<Ext, tyr::GroundTag>(state,
                                                                    builder,
                                                                    storage,
                                                                    sem::insert(denotations, empty_arguments).first,
                                                                    sem::insert(denotations, registers).first);
    const auto description = std::string(R"((q_atomic_state "fixed" (x y z)))");
    const auto query = parse_query(description, domain, *repository);
    const auto renamed = parse_query("(q_rename (u v w) " + description + ")", domain, *repository);
    const auto nested = parse_query("(q_rename (p q r) (q_rename (u v w) " + description + "))", domain, *repository);
    const auto original_rows = sem::evaluate(query, context);
    const auto renamed_rows = sem::evaluate(renamed, context);
    const auto nested_rows = sem::evaluate(nested, context);
    EXPECT_EQ(renamed_rows.get_storage_address(), original_rows.get_storage_address());
    EXPECT_EQ(nested_rows.get_storage_address(), original_rows.get_storage_address());
    EXPECT_EQ(sem::evaluate(renamed, context).get_storage_address(), original_rows.get_storage_address());
    EXPECT_TRUE(std::ranges::equal(original_rows.columns(), query.get_schema()));
    EXPECT_TRUE(std::ranges::equal(renamed_rows.columns(), renamed.get_schema()));
    EXPECT_TRUE(std::ranges::equal(nested_rows.columns(), nested.get_schema()));
    EXPECT_FALSE(std::ranges::equal(original_rows.columns(), renamed_rows.columns()));

    const auto truth = parse_query("(q_project () " + description + ")", domain, *repository);
    const auto true_rows = sem::evaluate(truth, context);
    EXPECT_TRUE(true_rows.columns().empty());
    EXPECT_EQ(true_rows.size(), 1);
    const auto true_join = parse_query("(q_join " + description + " (q_project () " + description + "))", domain, *repository);
    const auto false_join = parse_query("(q_join " + description + " (q_atomic_state \"missing\" ()))", domain, *repository);
    EXPECT_EQ(sem::evaluate(true_join, context).size(), original_rows.size());
    EXPECT_TRUE(sem::evaluate(false_join, context).empty());
    EXPECT_TRUE(std::ranges::equal(sem::evaluate(query, context).columns(), query.get_schema()));

    auto raw_rename_data = ygg::Data<dl::Query<Ext, dl::QueryRenameTag>>();
    raw_rename_data.arg = query.get_index();
    for (const auto column : query.get_columns())
        raw_rename_data.columns.push_back(column.get_index());
    std::swap(raw_rename_data.columns.front(), raw_rename_data.columns.back());
    auto raw_wrapper = ygg::Data<dl::Query<Ext>>(dl::insert(*repository, raw_rename_data).first.get_index());
    const auto raw = repository->insert(raw_wrapper).first;
    EXPECT_FALSE(raw.is_static());
    const auto raw_rows = sem::evaluate(raw, context);
    EXPECT_EQ(raw_rows.get_storage_address(), original_rows.get_storage_address());
    EXPECT_TRUE(caches.get_queries(false).contains(raw));

    auto raw_nested_data = ygg::Data<dl::Query<Ext, dl::QueryRenameTag>>();
    raw_nested_data.arg = raw.get_index();
    for (const auto column : nested.get_columns())
        raw_nested_data.columns.push_back(column.get_index());
    auto raw_nested_wrapper = ygg::Data<dl::Query<Ext>>(dl::insert(*repository, raw_nested_data).first.get_index());
    const auto raw_nested = repository->insert(raw_nested_wrapper).first;
    EXPECT_FALSE(raw_nested.is_static());
    const auto raw_nested_rows = sem::evaluate(raw_nested, context);
    EXPECT_EQ(raw_nested_rows.get_storage_address(), original_rows.get_storage_address());
    EXPECT_TRUE(std::ranges::equal(raw_nested_rows.columns(), raw_nested.get_schema()));
    EXPECT_TRUE(std::ranges::equal(raw_rows.columns(), raw.get_schema()));
    EXPECT_TRUE(std::ranges::equal(original_rows.columns(), query.get_schema()));
    EXPECT_TRUE(caches.get_queries(false).contains(raw_nested));

    storage.reset_all();
    EXPECT_FALSE(caches.get_queries(false).contains(raw));
    EXPECT_FALSE(caches.get_queries(false).contains(raw_nested));
    EXPECT_FALSE(caches.get_queries(true).contains(query));
    const auto rebuilt_nested = sem::evaluate(raw_nested, context);
    EXPECT_EQ(rebuilt_nested.size(), 2);
    EXPECT_TRUE(std::ranges::equal(rebuilt_nested.columns(), raw_nested.get_schema()));
    EXPECT_EQ(rebuilt_nested.get_storage_address(), sem::evaluate(query, context).get_storage_address());
    EXPECT_TRUE(caches.get_queries(false).contains(raw_nested));
    const auto rebuilt = sem::evaluate(raw, context);
    EXPECT_EQ(rebuilt.size(), 2);
    EXPECT_TRUE(std::ranges::equal(rebuilt.columns(), raw.get_schema()));
    EXPECT_EQ(rebuilt.get_storage_address(), sem::evaluate(query, context).get_storage_address());
    EXPECT_TRUE(caches.get_queries(false).contains(raw));
}

TEST(RunirQueries, QueryResultIdentityIncludesOrderedSchemaAndUnorderedRows)
{
    const auto search = query_search_context<tyr::GroundTag>();
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto builder = sem::Builder {};
    auto& repository = denotations.get_relation_repository();
    using Row = std::array<ObjectIndex, 2>;
    const auto intern = [&](std::array<ColumnIndex, 2> columns, std::array<Row, 2> rows)
    {
        auto result = builder.get_builder<ygg::database::Relation<ObjectIndex>>(columns);
        for (const auto& row : rows)
            result->insert(std::span<const ObjectIndex>(row));
        return ygg::database::insert(repository, *result).first;
    };
    const auto columns = std::array<ColumnIndex, 2> { ColumnIndex(0), ColumnIndex(1) };
    const auto rows = std::array<Row, 2> { Row { ObjectIndex(2), ObjectIndex(3) }, Row { ObjectIndex(4), ObjectIndex(5) } };
    const auto first = intern(columns, rows);
    EXPECT_EQ(intern(columns, rows), first);
    EXPECT_NE(intern({ ColumnIndex(1), ColumnIndex(0) }, rows), first);
    EXPECT_EQ(intern(columns, { rows[1], rows[0] }), first);
    EXPECT_EQ(repository.size(), 2);

    auto renamed_columns = std::array<ColumnIndex, 2> { ColumnIndex(6), ColumnIndex(7) };
    const auto renamed = repository.rename(first, renamed_columns);
    renamed_columns[0] = ColumnIndex(99);
    EXPECT_EQ(renamed.columns()[0], ColumnIndex(6));
    EXPECT_EQ(renamed.get_storage_address(), first.get_storage_address());
    EXPECT_NE(renamed, first);
    EXPECT_EQ(repository.rename(renamed, columns), first);
}

TEST(RunirQueries, PersistentRenameSurvivesIntermediateResetAndConstructorRelease)
{
    const auto search = query_search_context<tyr::GroundTag>();
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto constructors = dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = sem::Builder {};
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto intermediates = sem::EvaluationStorage<Ext>(denotations);
    auto output_memo = sem::DenotationCaches<Ext> {};
    auto arguments = ygg::Data<sem::CallArguments> {};
    auto registers = ygg::Data<sem::RegisterValues> {};
    auto context = sem::StateEvaluationContext<Ext, tyr::GroundTag>(state,
                                                                    builder,
                                                                    output_memo,
                                                                    denotations,
                                                                    intermediates,
                                                                    sem::insert(denotations, arguments).first,
                                                                    sem::insert(denotations, registers).first);
    const auto expression = parse_query(R"((q_rename (a b c) (q_atomic_state "triple" (x y z))))", domain, *constructors);
    const auto result = sem::evaluate(expression, context);
    ASSERT_EQ(result.size(), 4);
    const auto schema = std::vector<ColumnIndex>(result.columns().begin(), result.columns().end());
    const auto first = std::vector<ObjectIndex>(result.row(0).begin(), result.row(0).end());
    intermediates.reset_all();
    EXPECT_EQ(sem::evaluate(expression, context), result);
    output_memo.reset_all();
    constructors->clear();
    EXPECT_EQ(result.size(), 4);
    EXPECT_TRUE(std::ranges::equal(result.columns(), schema));
    EXPECT_TRUE(std::ranges::equal(result.row(0), first));
}

}  // namespace runir::tests
