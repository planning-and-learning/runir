#include "query_fixtures.hpp"

namespace runir::tests
{
namespace
{



}  // namespace

TEST(RunirQueries, StaticnessIsInferredAndSerializedWithoutChangingIdentity)
{
    const auto search = query_search_context<tyr::GroundTag>();
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
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
        check_metadata(kr::ps::ext::dl::parse_numerical("(n_count " + description + ")", domain, *repository), expected);
    }
    EXPECT_TRUE(kr::ps::ext::dl::parse_concept("(c_project x " + fixed + ")", domain, *repository).is_static());
    EXPECT_FALSE(kr::ps::ext::dl::parse_concept("(c_project x " + triple + ")", domain, *repository).is_static());
    EXPECT_TRUE(kr::ps::ext::dl::parse_role(R"((r_reflexive_transitive_closure (r_atomic_state "edge")))", domain, *repository).is_static());
    EXPECT_FALSE(kr::ps::ext::dl::parse_role(R"((r_reflexive_transitive_closure (r_register 0)))", domain, *repository).is_static());
    EXPECT_TRUE(kr::ps::ext::dl::parse_boolean("(b_nonempty " + fixed + ")", domain, *repository).is_static());
    EXPECT_TRUE(kr::ps::ext::dl::parse_boolean("(b_nonempty (q_generic_join (x y z) (x y z) " + fixed + " " + fixed + "))", domain, *repository).is_static());
    EXPECT_FALSE(kr::ps::ext::dl::parse_boolean("(b_nonempty (q_generic_join (x y z) (x y z) " + fixed + " " + triple + "))", domain, *repository).is_static());
    EXPECT_TRUE(kr::ps::ext::dl::parse_boolean("(b_nonempty (q_empty (x)))", domain, *repository).is_static());
    EXPECT_FALSE(kr::ps::ext::dl::parse_boolean("(b_nonempty (q_must_pass (x) (q_project (x y) " + triple + ") " + triple + " (q_project (x z) " + triple + R"() (q_concept p (c_top)))))",
                                                domain,
                                                *repository)
                     .is_static());
    EXPECT_FALSE(kr::ps::ext::dl::parse_boolean("(b_nonempty " + triple + ")", domain, *repository).is_static());
}
TEST(RunirQueries, QueryResultIdentityIncludesOrderedSchemaAndUnorderedRows)
{
    const auto search = query_search_context<tyr::GroundTag>();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder {};
    auto& repository = denotations.get_relation_repository();
    using Row = std::array<ObjectIndex, 2>;
    const auto intern = [&](std::array<ColumnIndex, 2> columns, std::array<Row, 2> rows)
    {
        auto result = builder.get_builder<ygg::database::Relation<QueryValues>>(columns);
        for (const auto& row : rows)
            result->insert(std::tuple { row[0], row[1] });
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
    EXPECT_EQ(renamed.columns()[0].label, ColumnIndex(6));
    EXPECT_EQ(renamed.get_storage_address(), first.get_storage_address());
    EXPECT_NE(renamed, first);
    EXPECT_EQ(repository.rename(renamed, columns), first);
}
TEST(RunirQueries, PersistentRenameSurvivesIntermediateResetAndConstructorRelease)
{
    const auto search = query_search_context<tyr::GroundTag>();
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto constructors = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder {};
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto intermediates = kr::dl::semantics::EvaluationStorage<Ext>(denotations);
    auto output_memo = kr::dl::semantics::DenotationCaches<Ext> {};
    auto arguments = ygg::Data<kr::dl::semantics::CallArguments> {};
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues> {};
    auto context = kr::dl::semantics::StateEvaluationContext<Ext, tyr::GroundTag>(state,
                                                                                  builder,
                                                                                  output_memo,
                                                                                  denotations,
                                                                                  intermediates,
                                                                                  kr::dl::semantics::insert(denotations, arguments).first,
                                                                                  kr::dl::semantics::insert(denotations, registers).first);
    const auto expression = parse_query(R"((q_rename (a b c) (q_atomic_state "triple" (x y z))))", domain, *constructors);
    const auto result = kr::dl::semantics::evaluate<tyr::GroundTag>(expression, context);
    ASSERT_EQ(result.size(), 4);
    const auto schema = column_labels(result.columns());
    const auto first = object_values(result[0]);
    intermediates.reset_all();
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(expression, context), result);
    output_memo.reset_all();
    constructors->clear();
    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(column_labels(result.columns()), schema);
    EXPECT_EQ(object_values(result[0]), first);
}

}  // namespace runir::tests
