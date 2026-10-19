#include "query_fixtures.hpp"

namespace runir::tests
{
namespace
{

template<kr::dl::FamilyTag Family>
void check_cached_queries()
{
    const auto search = query_search_context<tyr::GroundTag>();
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    auto factory = kr::dl::ConstructorRepositoryFactoryFor<Family>();
    auto repository = factory.create(search->task->get_repository());
    auto other_repository = factory.create(search->task->get_repository());
    auto& repo = *repository;
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage<Family>(denotations);
    auto& caches = storage.get_caches();
    auto context = kr::dl::semantics::StateEvaluationContext<Family, tyr::GroundTag>(state, builder, storage);

    auto column_data = ygg::Data<kr::dl::QueryColumn>();
    column_data.name = "x";
    const auto x = kr::dl::insert(repo, column_data).first.get_index();
    column_data.name = "renamed";
    const auto renamed_column = kr::dl::insert(repo, column_data).first.get_index();
    auto top_data = ygg::Data<kr::dl::Concept<Family, kr::dl::TopTag>>();
    auto top_wrapper = ygg::Data<kr::dl::Constructor<Family, kr::dl::ConceptTag>>(kr::dl::insert(repo, top_data).first.get_index());
    const auto top = kr::dl::insert(repo, top_wrapper).first;
    using Lift = kr::dl::Query<Family, kr::dl::QueryConceptTag>;
    auto lift_data = ygg::Data<Lift>();
    lift_data.arg = top.get_index();
    EXPECT_THROW(static_cast<void>(kr::dl::insert(repo, lift_data)), std::invalid_argument);
    EXPECT_EQ(repo.template size<Lift>(), 0);
    lift_data.columns.push_back(x);
    const auto [lift, created] = kr::dl::insert(repo, lift_data);
    EXPECT_TRUE(created);
    const auto [duplicate_lift, duplicate_created] = kr::dl::insert(repo, lift_data);
    EXPECT_FALSE(duplicate_created);
    EXPECT_EQ(duplicate_lift.get_index(), lift.get_index());
    EXPECT_EQ(repo.template size<Lift>(), 1);
    auto query_data = ygg::Data<kr::dl::Query<Family>>();
    query_data.variant = lift.get_index();
    const auto query = kr::dl::insert(repo, query_data).first;
    auto rename_data = ygg::Data<kr::dl::Query<Family, kr::dl::QueryRenameTag>>();
    rename_data.arg = query.get_index();
    rename_data.columns.push_back(renamed_column);
    query_data.variant = kr::dl::insert(repo, rename_data).first.get_index();
    const auto renamed = kr::dl::insert(repo, query_data).first;

    auto expected_columns = ygg::IndexList<kr::dl::QueryColumn>();
    expected_columns.push_back(renamed_column);
    expected_columns.push_back(x);
    const auto check_inferred_schema = [&](auto& data)
    {
        const auto concrete = kr::dl::insert(repo, data).first;
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
            stale.schema.clear();
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
        const auto wrapper = kr::dl::insert(repo, query_data).first;
        EXPECT_TRUE(std::ranges::equal(wrapper.get_columns(), expected_columns, {}, [](auto column) { return column.get_index(); }));
        return wrapper;
    };
    auto join_data = ygg::Data<kr::dl::Query<Family, kr::dl::QueryJoinTag>>();
    join_data.lhs = renamed.get_index();
    join_data.rhs = query.get_index();
    const auto joined = check_inferred_schema(join_data);
    join_data.lhs = joined.get_index();
    check_inferred_schema(join_data);  // The shared x column is not appended twice.
    check_relocated_data(join_data,
                         [&](const auto& decoded)
                         {
                             ygg::Builder<ygg::database::Relation<QueryValues>> lhs(decoded.plan.lhs_columns().span()),
                                 rhs(decoded.plan.rhs_columns().span()), result(decoded.plan.output_columns().span());
                             lhs.insert(std::tuple { ObjectIndex(10), ObjectIndex(20) });
                             lhs.insert(std::tuple { ObjectIndex(30), ObjectIndex(40) });
                             rhs.insert(std::tuple { ObjectIndex(20) });
                             ygg::database::Workspace<QueryValues> scratch;
                             ygg::database::join(lhs, rhs, decoded.plan, result, scratch);
                             EXPECT_EQ(result.size(), 1);
                             EXPECT_TRUE(result.contains(std::tuple { ObjectIndex(10), ObjectIndex(20) }));
                         });
    const auto slice_position = [](const ygg::database::ColumnSlice& slice) { return slice.offset / sizeof(ygg::uint_t); };
    const auto check_projection_plan = [&](auto& data, auto positions)
    {
        const auto concrete = kr::dl::insert(repo, data).first;
        check_relocated_data(concrete.get_data(),
                             [&](const auto& decoded)
                             {
                                 EXPECT_TRUE(std::ranges::equal(decoded.plan.positions(), positions, {}, slice_position));
                                 ygg::Builder<ygg::database::Relation<QueryValues>> input(decoded.plan.input_columns().span()),
                                     result(decoded.plan.output_columns().span());
                                 std::vector<ObjectIndex> row;
                                 for (size_t i = 0; i < input.arity(); ++i)
                                     row.emplace_back(10 + i);
                                 std::vector<std::byte> bytes;
                                 input.insert(ygg::database::encode_row<QueryValues, ObjectIndex>(row, bytes));
                                 ygg::database::Workspace<QueryValues> scratch;
                                 ygg::database::project(input, decoded.plan, result, scratch);
                                 ASSERT_EQ(result.size(), 1);
                                 for (size_t i = 0; i < positions.size(); ++i)
                                     EXPECT_EQ(result[0].get<ObjectIndex>(i), ObjectIndex(10 + positions[i]));
                             });
        auto stale = data;
        stale.plan = {};
        EXPECT_EQ(stale, concrete.get_data());
        const auto [same, created] = kr::dl::insert(repo, stale);
        EXPECT_FALSE(created);
        EXPECT_EQ(same.get_index(), concrete.get_index());
        EXPECT_TRUE(std::ranges::equal(stale.plan.positions(), positions, {}, slice_position));
        stale.clear();
        EXPECT_TRUE(stale.plan.input_columns().empty());
        EXPECT_TRUE(stale.plan.output_columns().empty());
        EXPECT_TRUE(stale.plan.positions().empty());
    };
    auto project_data = ygg::Data<kr::dl::Query<Family, kr::dl::QueryProjectTag>>();
    project_data.arg = joined.get_index();
    project_data.columns.push_back(x);
    project_data.columns.push_back(renamed_column);
    check_projection_plan(project_data, std::array<size_t, 2> { 1, 0 });
    auto role_projection_data = ygg::Data<kr::dl::QueryProjection<Family, kr::dl::RoleTag>>();
    role_projection_data.arg = project_data.arg;
    role_projection_data.columns = project_data.columns;
    check_projection_plan(role_projection_data, std::array<size_t, 2> { 1, 0 });

    auto union_data = ygg::Data<kr::dl::Query<Family, kr::dl::QueryUnionTag>>();
    union_data.lhs = joined.get_index();
    union_data.rhs = joined.get_index();
    check_inferred_schema(union_data);
    auto difference_data = ygg::Data<kr::dl::Query<Family, kr::dl::QueryDifferenceTag>>();
    difference_data.lhs = joined.get_index();
    difference_data.rhs = joined.get_index();
    check_inferred_schema(difference_data);
    auto equal_data = ygg::Data<kr::dl::Query<Family, kr::dl::QuerySelectEqualTag>>();
    equal_data.arg = joined.get_index();
    equal_data.lhs_column = x;
    equal_data.rhs_column = renamed_column;
    check_inferred_schema(equal_data);
    auto value_data = ygg::Data<kr::dl::Query<Family, kr::dl::QuerySelectValueTag>>();
    value_data.arg = joined.get_index();
    value_data.column = x;
    value_data.object = search->task->get_domain().get_domain().get_constants()[0].get_index();
    check_inferred_schema(value_data);

    auto count_data = ygg::Data<kr::dl::Numerical<Family, kr::dl::CountTag>>(renamed.get_index());
    auto count_wrapper = ygg::Data<kr::dl::Constructor<Family, kr::dl::NumericalTag>>(kr::dl::insert(repo, count_data).first.get_index());
    const auto count = kr::dl::insert(repo, count_wrapper).first;
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(count, context).get(), 3);
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(count, context).get(), 3);
    EXPECT_TRUE(caches.template get<kr::dl::ConceptTag>(true).contains(top));

    auto projection_data = ygg::Data<kr::dl::QueryProjection<Family, kr::dl::ConceptTag>>();
    projection_data.arg = renamed.get_index();
    projection_data.columns.push_back(renamed_column);
    check_projection_plan(projection_data, std::array<size_t, 1> { 0 });
    auto projection_wrapper = ygg::Data<kr::dl::Constructor<Family, kr::dl::ConceptTag>>(kr::dl::insert(repo, projection_data).first.get_index());
    const auto projection = kr::dl::insert(repo, projection_wrapper).first;
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(projection, context).get().count(), 3);

    const auto held = kr::dl::semantics::evaluate<tyr::GroundTag>(renamed, context);
    const auto* held_storage = held.get_storage_address();
    const auto* held_columns = held.columns().data();
    EXPECT_EQ(held.columns()[0].label, ColumnIndex(ygg::uint_t(renamed_column)));
    EXPECT_NE(held_columns, renamed.get_schema().data());
    EXPECT_TRUE(std::ranges::equal(held.columns(), renamed.get_schema()));
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(query, context).get_storage_address(), held_storage);
    auto nested_rename_data = rename_data;
    nested_rename_data.arg = renamed.get_index();
    nested_rename_data.columns[0] = x;
    query_data.variant = kr::dl::insert(repo, nested_rename_data).first.get_index();
    const auto nested = kr::dl::semantics::evaluate<tyr::GroundTag>(kr::dl::insert(repo, query_data).first, context);
    EXPECT_EQ(nested.columns()[0].label, ColumnIndex(ygg::uint_t(x)));
    EXPECT_EQ(nested.get_storage_address(), held_storage);
    EXPECT_EQ(nested.size(), held.size());
    EXPECT_EQ(held.columns()[0].label, ColumnIndex(ygg::uint_t(renamed_column)));
    // Owned labels and shared rows remain valid during repository/cache growth.
    for (int i = 0; i < 64; ++i)
    {
        column_data.name = "extra_" + std::to_string(i);
        const auto extra_column = kr::dl::insert(repo, column_data).first.get_index();
        rename_data.columns[0] = extra_column;
        query_data.variant = kr::dl::insert(repo, rename_data).first.get_index();
        const auto extra_query = kr::dl::insert(repo, query_data).first;
        const auto other = kr::dl::semantics::evaluate<tyr::GroundTag>(extra_query, context);
        EXPECT_EQ(other.get_storage_address(), held_storage);
        EXPECT_EQ(held.size(), 3);
        EXPECT_EQ(held.columns().data(), held_columns);
        EXPECT_EQ(held.columns()[0].label, ColumnIndex(ygg::uint_t(renamed_column)));
        EXPECT_EQ(other.columns()[0].label, ColumnIndex(ygg::uint_t(extra_column)));
    }

    // Repositories from one factory have distinct identities even when query indices match.
    auto& other_repo = *other_repository;
    EXPECT_NE(other_repo.get_index(), repo.get_index());
    column_data.name = "unused";
    static_cast<void>(kr::dl::insert(other_repo, column_data));
    column_data.name = "other";
    const auto other_column = kr::dl::insert(other_repo, column_data).first.get_index();
    top_wrapper.variant = kr::dl::insert(other_repo, top_data).first.get_index();
    lift_data.arg = kr::dl::insert(other_repo, top_wrapper).first.get_index();
    lift_data.columns[0] = other_column;
    query_data.variant = kr::dl::insert(other_repo, lift_data).first.get_index();
    const auto other_query = kr::dl::insert(other_repo, query_data).first;
    ASSERT_EQ(other_query.get_index(), query.get_index());
    ASSERT_NE(other_column, x);
    const auto cached = kr::dl::semantics::evaluate<tyr::GroundTag>(query, context);
    const auto other_cached = kr::dl::semantics::evaluate<tyr::GroundTag>(other_query, context);
    EXPECT_EQ(cached.columns()[0].label, ColumnIndex(ygg::uint_t(x)));
    EXPECT_EQ(other_cached.columns()[0].label, ColumnIndex(ygg::uint_t(other_column)));
    EXPECT_EQ(cached.size(), 3);
    EXPECT_EQ(other_cached.size(), 3);
    EXPECT_NE(cached, other_cached);
    EXPECT_EQ(cached.get_storage_address(), other_cached.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(query, context).get_storage_address(), cached.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(other_query, context).get_storage_address(), other_cached.get_storage_address());

    // Clear invalidates the borrowed views; reevaluation reconstructs rows and schemas.
    storage.reset_all();
    EXPECT_TRUE(caches.get_queries(true).empty());
    EXPECT_TRUE(caches.get_queries(false).empty());
    const auto rebuilt = kr::dl::semantics::evaluate<tyr::GroundTag>(renamed, context);
    EXPECT_EQ(rebuilt.size(), 3);
    EXPECT_EQ(rebuilt.columns()[0].label, ColumnIndex(ygg::uint_t(renamed_column)));
    EXPECT_EQ(rebuilt.get_storage_address(), kr::dl::semantics::evaluate<tyr::GroundTag>(query, context).get_storage_address());
    storage.reset_all();
    repo.clear();
}

}  // namespace

TEST(RunirQueries, BaseCachingAndBorrowedRenameLifetime) { check_cached_queries<kr::BaseFamilyTag>(); }
TEST(RunirQueries, UnsInheritsQueryEvaluation) { check_cached_queries<kr::UnsFamilyTag>(); }

}  // namespace runir::tests
