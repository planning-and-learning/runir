#include "planning_fixtures.hpp"

#include <algorithm>
#include <array>
#include <cista/serialization.h>
#include <filesystem>
#include <gtest/gtest.h>
#include <runir/kr/dl/query_data.hpp>
#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/base/state_evaluation_context.hpp>
#include <runir/kr/dl/semantics/evaluation.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/dl/semantics/uns/state_evaluation_context.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/database/operations.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace runir::tests
{
namespace
{
namespace dl = kr::dl;
namespace sem = dl::semantics;
namespace parser = kr::ps::ext::dl;
using Ext = kr::ExtFamilyTag;

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

template<tyr::TaskKind Kind>
void check_queries()
{
    const auto search = query_search_context<Kind>();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = sem::Builder();
    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto caches = sem::DenotationCaches<Ext>();
    auto empty_arguments = ygg::Data<sem::CallArguments>();
    auto registers = ygg::Data<sem::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = sem::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                          builder,
                                                          denotations,
                                                          builder.get_workspace(),
                                                          caches,
                                                          sem::get_or_create(denotations, empty_arguments).first,
                                                          sem::get_or_create(denotations, registers).first);

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
        caches.clear();
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
                                                              denotations,
                                                              builder.get_workspace(),
                                                              caches,
                                                              sem::get_or_create(denotations, empty_arguments).first,
                                                              sem::get_or_create(denotations, registers).first);
        return sem::evaluate(expression, updated);
    };
    const auto register_count = parser::parse_numerical("(n_count (q_join " + triple + " (q_concept x (c_register 0))))", domain, *repository);
    EXPECT_EQ(evaluate_register(register_count).get(), 0);
    registers.concept_values[0] = a.get_index();
    caches.clear(false);
    EXPECT_EQ(evaluate_register(register_count).get(), 2);
    registers.concept_values[0] = c.get_index();
    caches.clear(false);
    EXPECT_EQ(evaluate_register(register_count).get(), 0);
    registers.concept_values[0].reset();
    caches.clear(false);
    EXPECT_EQ(evaluate_register(register_count).get(), 0);

    const auto role_register_count = parser::parse_numerical("(n_count (q_join " + triple + " (q_role (x y) (r_register 0))))", domain, *repository);
    registers.role_values[0] = ::cista::pair(a.get_index(), b.get_index());
    caches.clear(false);
    EXPECT_EQ(evaluate_register(role_register_count).get(), 2);
    registers.role_values[0] = ::cista::pair(b.get_index(), c.get_index());
    caches.clear(false);
    EXPECT_EQ(evaluate_register(role_register_count).get(), 1);

    EXPECT_FALSE(context.registers().template get<dl::ConceptTag>()[0]);
    EXPECT_FALSE(context.registers().template get<dl::RoleTag>()[0]);

    const auto a_set = sem::evaluate(parser::parse_concept(R"((c_nominal "a"))", domain, *repository), context);
    const auto c_set = sem::evaluate(parser::parse_concept(R"((c_nominal "c"))", domain, *repository), context);
    auto argument_values = ygg::Data<sem::CallArguments>();
    argument_values.concept_arguments.push_back(a_set.get_index());
    auto argument_context = sem::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                                   builder,
                                                                   denotations,
                                                                   builder.get_workspace(),
                                                                   caches,
                                                                   sem::get_or_create(denotations, argument_values).first,
                                                                   sem::get_or_create(denotations, registers).first);
    const auto argument_count = parser::parse_numerical("(n_count (q_join " + triple + " (q_concept x (c_argument 0))))", domain, *repository);
    // An invalid Ext argument must propagate through DL/query variant dispatch, not terminate.
    EXPECT_THROW(sem::evaluate(argument_count, context), std::out_of_range);
    caches.clear(false);
    EXPECT_EQ(sem::evaluate(argument_count, argument_context).get(), 2);
    argument_values.concept_arguments[0] = c_set.get_index();
    auto other_argument_context = sem::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                                         builder,
                                                                         denotations,
                                                                         builder.get_workspace(),
                                                                         caches,
                                                                         sem::get_or_create(denotations, argument_values).first,
                                                                         sem::get_or_create(denotations, registers).first);
    caches.clear(false);
    EXPECT_EQ(sem::evaluate(argument_count, other_argument_context).get(), 0);
    EXPECT_EQ(argument_context.arguments().template get<dl::ConceptTag>()[0].get_index(), a_set.get_index());

    const auto successors = search->successor_generator->get_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator);
    ASSERT_FALSE(successors.empty());
    auto next = sem::StateEvaluationContext<Ext, Kind>(successors.front().get_state(),
                                                       builder,
                                                       denotations,
                                                       builder.get_workspace(),
                                                       caches,
                                                       sem::get_or_create(denotations, empty_arguments).first,
                                                       sem::get_or_create(denotations, registers).first);
    const auto triple_count = parser::parse_numerical("(n_count " + triple + ")", domain, *repository);
    const auto derived_count = parser::parse_numerical(R"((n_count (q_atomic_state "copied" (x y z))))", domain, *repository);
    const auto ready_test = parser::parse_boolean("(b_nonempty " + ready + ")", domain, *repository);
    caches.clear(false);
    EXPECT_EQ(sem::evaluate(triple_count, next).get(), 3);
    EXPECT_EQ(sem::evaluate(derived_count, next).get(), 3);
    EXPECT_FALSE(sem::evaluate(ready_test, next).get());
    caches.clear(false);
    EXPECT_EQ(sem::evaluate(triple_count, context).get(), 4);
    EXPECT_TRUE(sem::evaluate(ready_test, context).get());
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
    auto caches = sem::DenotationCaches<Family>();
    auto context = sem::StateEvaluationContext<Family, tyr::GroundTag>(state, builder, denotations, builder.get_workspace(), caches);

    auto column_data = ygg::Data<dl::QueryColumn>();
    column_data.name = "x";
    const auto x = dl::get_or_create(repo, column_data).first.get_index();
    column_data.name = "renamed";
    const auto renamed_column = dl::get_or_create(repo, column_data).first.get_index();
    auto top_data = ygg::Data<dl::Concept<Family, dl::TopTag>>();
    auto top_wrapper = ygg::Data<dl::Constructor<Family, dl::ConceptTag>>(dl::get_or_create(repo, top_data).first.get_index());
    const auto top = dl::get_or_create(repo, top_wrapper).first;
    using Lift = dl::Query<Family, dl::QueryConceptTag>;
    auto lift_data = ygg::Data<Lift>();
    lift_data.arg = top.get_index();
    EXPECT_THROW(static_cast<void>(dl::get_or_create(repo, lift_data)), std::invalid_argument);
    EXPECT_EQ(repo.template size<Lift>(), 0);
    lift_data.columns.push_back(x);
    const auto [lift, created] = dl::get_or_create(repo, lift_data);
    EXPECT_TRUE(created);
    const auto [duplicate_lift, duplicate_created] = dl::get_or_create(repo, lift_data);
    EXPECT_FALSE(duplicate_created);
    EXPECT_EQ(duplicate_lift.get_index(), lift.get_index());
    EXPECT_EQ(repo.template size<Lift>(), 1);
    auto query_data = ygg::Data<dl::Query<Family>>();
    query_data.variant = lift.get_index();
    const auto query = dl::get_or_create(repo, query_data).first;
    auto rename_data = ygg::Data<dl::Query<Family, dl::QueryRenameTag>>();
    rename_data.arg = query.get_index();
    rename_data.columns.push_back(renamed_column);
    query_data.variant = dl::get_or_create(repo, rename_data).first.get_index();
    const auto renamed = dl::get_or_create(repo, query_data).first;

    auto expected_columns = ygg::IndexList<dl::QueryColumn>();
    expected_columns.push_back(renamed_column);
    expected_columns.push_back(x);
    const auto check_inferred_schema = [&](auto& data)
    {
        const auto concrete = dl::get_or_create(repo, data).first;
        EXPECT_EQ(concrete.get_data().columns, expected_columns);

        check_relocated_data(concrete.get_data(),
                             [&](const auto& decoded)
                             {
                                 EXPECT_EQ(decoded.columns, expected_columns);
                                 if constexpr (requires { decoded.schema; })
                                 {
                                     EXPECT_TRUE(std::ranges::equal(decoded.schema.view(), concrete.get_schema()));
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
        const auto [same, created] = dl::get_or_create(repo, stale);
        EXPECT_FALSE(created);
        EXPECT_EQ(same.get_index(), concrete.get_index());
        EXPECT_EQ(stale.columns, expected_columns);
        if constexpr (requires { stale.schema; })
        {
            EXPECT_TRUE(std::ranges::equal(stale.schema.view(), concrete.get_schema()));
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
        const auto wrapper = dl::get_or_create(repo, query_data).first;
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
                             ygg::database::Relation<> lhs(decoded.plan.lhs_columns()), rhs(decoded.plan.rhs_columns()), result(decoded.plan.output_columns());
                             lhs.insert({ 10, 20 });
                             lhs.insert({ 30, 40 });
                             rhs.insert({ 20 });
                             ygg::database::Workspace<> scratch;
                             ygg::database::join(lhs.view(), rhs.view(), decoded.plan, result, scratch);
                             EXPECT_EQ(result.size(), 1);
                             EXPECT_TRUE(result.contains({ 10, 20 }));
                         });
    const auto check_projection_plan = [&](auto& data, auto positions)
    {
        const auto concrete = dl::get_or_create(repo, data).first;
        check_relocated_data(concrete.get_data(),
                             [&](const auto& decoded)
                             {
                                 EXPECT_TRUE(std::ranges::equal(decoded.plan.positions(), positions));
                                 ygg::database::Relation<> input(decoded.plan.input_columns()), result(decoded.plan.output_columns());
                                 std::vector<ygg::uint_t> row(input.arity());
                                 for (size_t i = 0; i < row.size(); ++i)
                                     row[i] = 10 + i;
                                 input.insert(row);
                                 ygg::database::Workspace<> scratch;
                                 ygg::database::project(input.view(), decoded.plan, result, scratch);
                                 ASSERT_EQ(result.size(), 1);
                                 for (size_t i = 0; i < positions.size(); ++i)
                                     EXPECT_EQ(result[0][i], 10 + positions[i]);
                             });
        auto stale = data;
        stale.plan = {};
        EXPECT_EQ(stale, concrete.get_data());
        const auto [same, created] = dl::get_or_create(repo, stale);
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
    auto count_wrapper = ygg::Data<dl::Constructor<Family, dl::NumericalTag>>(dl::get_or_create(repo, count_data).first.get_index());
    const auto count = dl::get_or_create(repo, count_wrapper).first;
    EXPECT_EQ(sem::evaluate(count, context).get(), 3);
    EXPECT_EQ(sem::evaluate(count, context).get(), 3);
    EXPECT_TRUE(caches.template get<dl::ConceptTag>(true).contains(top));

    auto projection_data = ygg::Data<dl::QueryProjection<Family, dl::ConceptTag>>();
    projection_data.arg = renamed.get_index();
    projection_data.columns.push_back(renamed_column);
    check_projection_plan(projection_data, std::array<size_t, 1> { 0 });
    auto projection_wrapper = ygg::Data<dl::Constructor<Family, dl::ConceptTag>>(dl::get_or_create(repo, projection_data).first.get_index());
    const auto projection = dl::get_or_create(repo, projection_wrapper).first;
    EXPECT_EQ(sem::evaluate(projection, context).get().count(), 3);

    const auto held = sem::evaluate(renamed, context);
    const auto* held_storage = &held.storage();
    const auto* held_columns = held.columns().data();
    EXPECT_EQ(held.columns()[0], ygg::uint_t(renamed_column));
    EXPECT_EQ(held_columns, renamed.get_schema().data());
    EXPECT_EQ(&sem::evaluate(query, context).storage(), held_storage);
    auto nested_rename_data = rename_data;
    nested_rename_data.arg = renamed.get_index();
    nested_rename_data.columns[0] = x;
    query_data.variant = dl::get_or_create(repo, nested_rename_data).first.get_index();
    const auto nested = sem::evaluate(dl::get_or_create(repo, query_data).first, context);
    EXPECT_EQ(nested.columns()[0], ygg::uint_t(x));
    EXPECT_EQ(&nested.storage(), held_storage);
    EXPECT_EQ(nested.size(), held.size());
    EXPECT_EQ(held.columns()[0], ygg::uint_t(renamed_column));
    // Borrowed labels and shared rows remain valid during repository/cache growth.
    for (int i = 0; i < 64; ++i)
    {
        column_data.name = "extra_" + std::to_string(i);
        const auto extra_column = dl::get_or_create(repo, column_data).first.get_index();
        rename_data.columns[0] = extra_column;
        query_data.variant = dl::get_or_create(repo, rename_data).first.get_index();
        const auto extra_query = dl::get_or_create(repo, query_data).first;
        const auto other = sem::evaluate(extra_query, context);
        EXPECT_EQ(&other.storage(), held_storage);
        EXPECT_EQ(held.size(), 3);
        EXPECT_EQ(held.columns().data(), held_columns);
        EXPECT_EQ(held.columns()[0], ygg::uint_t(renamed_column));
        EXPECT_EQ(other.columns()[0], ygg::uint_t(extra_column));
    }

    // Repositories from one factory have distinct identities even when query indices match.
    auto& other_repo = *other_repository;
    EXPECT_NE(other_repo.get_index(), repo.get_index());
    column_data.name = "unused";
    static_cast<void>(dl::get_or_create(other_repo, column_data));
    column_data.name = "other";
    const auto other_column = dl::get_or_create(other_repo, column_data).first.get_index();
    top_wrapper.variant = dl::get_or_create(other_repo, top_data).first.get_index();
    lift_data.arg = dl::get_or_create(other_repo, top_wrapper).first.get_index();
    lift_data.columns[0] = other_column;
    query_data.variant = dl::get_or_create(other_repo, lift_data).first.get_index();
    const auto other_query = dl::get_or_create(other_repo, query_data).first;
    ASSERT_EQ(other_query.get_index(), query.get_index());
    ASSERT_NE(other_column, x);
    const auto cached = sem::evaluate(query, context);
    const auto other_cached = sem::evaluate(other_query, context);
    EXPECT_EQ(cached.columns()[0], ygg::uint_t(x));
    EXPECT_EQ(other_cached.columns()[0], ygg::uint_t(other_column));
    EXPECT_EQ(cached.size(), 3);
    EXPECT_EQ(other_cached.size(), 3);
    EXPECT_NE(&cached.storage(), &other_cached.storage());
    EXPECT_EQ(&sem::evaluate(query, context).storage(), &cached.storage());
    EXPECT_EQ(&sem::evaluate(other_query, context).storage(), &other_cached.storage());

    // Clear invalidates the borrowed views; reevaluation reconstructs rows and schemas.
    caches.clear();
    EXPECT_TRUE(caches.get_queries(true).empty());
    EXPECT_TRUE(caches.get_queries(false).empty());
    const auto rebuilt = sem::evaluate(renamed, context);
    EXPECT_EQ(rebuilt.size(), 3);
    EXPECT_EQ(rebuilt.columns()[0], ygg::uint_t(renamed_column));
    EXPECT_EQ(&rebuilt.storage(), &sem::evaluate(query, context).storage());
    caches.clear();
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
    auto caches = sem::DenotationCaches<Ext>();
    auto empty_arguments = ygg::Data<sem::CallArguments>();
    auto registers = ygg::Data<sem::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = sem::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                          builder,
                                                          denotations,
                                                          builder.get_workspace(),
                                                          caches,
                                                          sem::get_or_create(denotations, empty_arguments).first,
                                                          sem::get_or_create(denotations, registers).first);
    const auto fixed = parse_query(R"((q_atomic_state "fixed" (x y z)))", domain, *repository);
    const auto goal = parse_query(R"((q_atomic_goal "triple" false (x y z)))", domain, *repository);
    const auto derived = parse_query(R"((q_atomic_state "copied" (x y z)))", domain, *repository);
    const auto ready = parse_query(R"((q_atomic_state "ready" ()))", domain, *repository);
    const auto count = parser::parse_numerical(R"((n_count (q_atomic_state "triple" (x y z))))", domain, *repository);
    const auto fixed_count = parser::parse_numerical(R"((n_count (q_atomic_state "fixed" (x y z))))", domain, *repository);

    const auto fixed_rows = sem::evaluate(fixed, context);
    const auto goal_rows = sem::evaluate(goal, context);
    EXPECT_EQ(fixed_rows.size(), 2);
    EXPECT_EQ(goal_rows.size(), 1);
    EXPECT_EQ(&sem::evaluate(fixed, context).storage(), &fixed_rows.storage());
    EXPECT_EQ(sem::evaluate(derived, context).size(), 4);
    EXPECT_EQ(sem::evaluate(ready, context).size(), 1);
    EXPECT_EQ(sem::evaluate(count, context).get(), 4);
    EXPECT_EQ(sem::evaluate(fixed_count, context).get(), 2);

    const auto successors = search->successor_generator->get_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator);
    ASSERT_FALSE(successors.empty());
    auto next = sem::StateEvaluationContext<Ext, Kind>(successors.front().get_state(),
                                                       builder,
                                                       denotations,
                                                       builder.get_workspace(),
                                                       caches,
                                                       sem::get_or_create(denotations, empty_arguments).first,
                                                       sem::get_or_create(denotations, registers).first);
    caches.clear(false);
    EXPECT_TRUE(caches.get_queries(false).empty());
    EXPECT_TRUE(caches.template get<dl::NumericalTag>(false).empty());
    EXPECT_TRUE(caches.get_queries(true).contains(fixed));
    EXPECT_TRUE(caches.template get<dl::NumericalTag>(true).contains(fixed_count));
    EXPECT_EQ(&sem::evaluate(fixed, next).storage(), &fixed_rows.storage());
    EXPECT_EQ(&sem::evaluate(goal, next).storage(), &goal_rows.storage());
    EXPECT_EQ(sem::evaluate(derived, next).size(), 3);
    EXPECT_TRUE(sem::evaluate(ready, next).empty());
    EXPECT_EQ(sem::evaluate(count, next).get(), 3);

    caches.clear(true);
    EXPECT_TRUE(caches.get_queries(true).empty());
    EXPECT_TRUE(caches.template get<dl::NumericalTag>(true).empty());
    EXPECT_TRUE(caches.get_queries(false).empty());
    EXPECT_TRUE(caches.template get<dl::NumericalTag>(false).empty());
    caches.clear();
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
    auto caches = sem::DenotationCaches<Ext>();
    auto empty_arguments = ygg::Data<sem::CallArguments>();
    auto registers = ygg::Data<sem::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = sem::StateEvaluationContext<Ext, Kind>(state,
                                                          builder,
                                                          denotations,
                                                          builder.get_workspace(),
                                                          caches,
                                                          sem::get_or_create(denotations, empty_arguments).first,
                                                          sem::get_or_create(denotations, registers).first);
    const auto a = domain.get_constants()[0];
    const auto b = domain.get_constants()[1];
    const auto c = domain.get_constants()[2];
    const auto a_set = sem::evaluate(parser::parse_concept(R"((c_nominal "a"))", domain, *repository), context);
    const auto b_set = sem::evaluate(parser::parse_concept(R"((c_nominal "b"))", domain, *repository), context);
    auto argument_values = ygg::Data<sem::CallArguments>();
    argument_values.concept_arguments.push_back(a_set.get_index());
    const auto make_bound = [&]
    {
        return sem::StateEvaluationContext<Ext, Kind>(state,
                                                      builder,
                                                      denotations,
                                                      builder.get_workspace(),
                                                      caches,
                                                      sem::get_or_create(denotations, argument_values).first,
                                                      sem::get_or_create(denotations, registers).first);
    };
    registers.concept_values[0] = a.get_index();
    registers.role_values[0] = ::cista::pair(a.get_index(), b.get_index());
    auto bound = make_bound();
    const auto fixed = parse_query(R"((q_atomic_state "fixed" (x y z)))", domain, *repository);
    const auto register_count =
        parser::parse_numerical(R"((n_count (q_join (q_atomic_state "fixed" (x y z)) (q_concept x (c_register 0)))))", domain, *repository);
    const auto argument = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_concept x (c_argument 0))))", domain, *repository);
    const auto role_register = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_role (x y) (r_register 0))))", domain, *repository);

    const auto fixed_rows = sem::evaluate(fixed, bound);
    EXPECT_EQ(sem::evaluate(register_count, bound).get(), 1);
    EXPECT_EQ(sem::evaluate(argument, bound).size(), 1);
    EXPECT_EQ(sem::evaluate(role_register, bound).size(), 1);
    registers.concept_values[0] = b.get_index();
    registers.role_values[0] = ::cista::pair(b.get_index(), c.get_index());
    argument_values.concept_arguments[0] = b_set.get_index();
    auto rebound = make_bound();
    caches.clear(false);
    EXPECT_EQ(&sem::evaluate(fixed, rebound).storage(), &fixed_rows.storage());
    EXPECT_EQ(sem::evaluate(register_count, rebound).get(), 0);
    EXPECT_TRUE(sem::evaluate(argument, rebound).empty());
    EXPECT_TRUE(sem::evaluate(role_register, rebound).empty());
    EXPECT_TRUE(caches.template get<dl::NumericalTag>(false).contains(register_count));
    EXPECT_FALSE(caches.template get<dl::NumericalTag>(true).contains(register_count));

    registers.concept_values[0].reset();
    registers.role_values[0].reset();
    argument_values.concept_arguments[0] = a_set.get_index();
    auto cleared = make_bound();
    caches.clear(false);
    EXPECT_EQ(sem::evaluate(register_count, cleared).get(), 0);
    EXPECT_EQ(sem::evaluate(argument, cleared).size(), 1);
    EXPECT_TRUE(sem::evaluate(role_register, cleared).empty());
    EXPECT_EQ(bound.registers().template get<dl::ConceptTag>()[0].value().get_index(), a.get_index());
    EXPECT_EQ(bound.arguments().template get<dl::ConceptTag>()[0].get_index(), a_set.get_index());
    EXPECT_EQ(rebound.registers().template get<dl::ConceptTag>()[0].value().get_index(), b.get_index());
    EXPECT_EQ(rebound.arguments().template get<dl::ConceptTag>()[0].get_index(), b_set.get_index());
}

}  // namespace

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
        const auto [same, created] = dl::get_or_create(*repository, stale);
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
    auto caches = sem::DenotationCaches<Ext>();
    auto empty_arguments = ygg::Data<sem::CallArguments>();
    auto registers = ygg::Data<sem::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = sem::StateEvaluationContext<Ext, tyr::GroundTag>(state,
                                                                    builder,
                                                                    denotations,
                                                                    builder.get_workspace(),
                                                                    caches,
                                                                    sem::get_or_create(denotations, empty_arguments).first,
                                                                    sem::get_or_create(denotations, registers).first);
    const auto description = std::string(R"((q_atomic_state "fixed" (x y z)))");
    const auto query = parse_query(description, domain, *repository);
    const auto renamed = parse_query("(q_rename (u v w) " + description + ")", domain, *repository);
    const auto nested = parse_query("(q_rename (p q r) (q_rename (u v w) " + description + "))", domain, *repository);
    const auto original_rows = sem::evaluate(query, context);
    const auto renamed_rows = sem::evaluate(renamed, context);
    const auto nested_rows = sem::evaluate(nested, context);
    EXPECT_EQ(&renamed_rows.storage(), &original_rows.storage());
    EXPECT_EQ(&nested_rows.storage(), &original_rows.storage());
    EXPECT_EQ(&sem::evaluate(renamed, context).storage(), &original_rows.storage());
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
    auto raw_wrapper = ygg::Data<dl::Query<Ext>>(dl::get_or_create(*repository, raw_rename_data).first.get_index());
    const auto raw = repository->get_or_create(raw_wrapper).first;
    EXPECT_FALSE(raw.is_static());
    const auto raw_rows = sem::evaluate(raw, context);
    EXPECT_EQ(&raw_rows.storage(), &original_rows.storage());
    EXPECT_TRUE(caches.get_queries(false).contains(raw));
    caches.clear(true);
    EXPECT_FALSE(caches.get_queries(false).contains(raw));
    EXPECT_FALSE(caches.get_queries(true).contains(query));
    const auto rebuilt = sem::evaluate(raw, context);
    EXPECT_EQ(rebuilt.size(), 2);
    EXPECT_TRUE(std::ranges::equal(rebuilt.columns(), raw.get_schema()));
    EXPECT_EQ(&rebuilt.storage(), &sem::evaluate(query, context).storage());
    EXPECT_TRUE(caches.get_queries(false).contains(raw));
}

}  // namespace runir::tests
