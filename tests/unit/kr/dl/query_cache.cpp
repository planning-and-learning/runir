#include "query_fixtures.hpp"

namespace runir::tests
{
namespace
{

template<tyr::TaskKind Kind>
void check_query_cache_across_states()
{
    const auto search = query_search_context<Kind>();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage<Ext>(denotations);
    auto& caches = storage.get_caches();
    auto empty_arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = kr::dl::semantics::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                                        builder,
                                                                        storage,
                                                                        kr::dl::semantics::insert(denotations, empty_arguments).first,
                                                                        kr::dl::semantics::insert(denotations, registers).first);
    const auto fixed = parse_query(R"((q_atomic_state "fixed" (x y z)))", domain, *repository);
    const auto goal = parse_query(R"((q_atomic_goal "triple" false (x y z)))", domain, *repository);
    const auto derived = parse_query(R"((q_atomic_state "copied" (x y z)))", domain, *repository);
    const auto ready = parse_query(R"((q_atomic_state "ready" ()))", domain, *repository);
    const auto triple = parse_query(R"((q_atomic_state "triple" (x y z)))", domain, *repository);
    const auto fixed_join = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_atomic_state "triple" (x y z))))", domain, *repository);
    const auto reversed_join = parse_query(R"((q_join (q_atomic_state "triple" (x y z)) (q_atomic_state "fixed" (x y z))))", domain, *repository);
    const auto static_join = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_atomic_state "fixed" (x y z))))", domain, *repository);
    const auto count = kr::ps::ext::dl::parse_numerical(R"((n_count (q_atomic_state "triple" (x y z))))", domain, *repository);
    const auto fixed_count = kr::ps::ext::dl::parse_numerical(R"((n_count (q_atomic_state "fixed" (x y z))))", domain, *repository);
    const auto matching_row =
        std::tuple { domain.get_constants()[0].get_index(), domain.get_constants()[1].get_index(), domain.get_constants()[2].get_index() };
    const auto check_mixed_joins = [&](auto& target)
    {
        const auto matches = kr::dl::semantics::evaluate<Kind>(triple, target).contains(matching_row);
        for (const auto query : { fixed_join, reversed_join })
        {
            const auto rows = kr::dl::semantics::evaluate<Kind>(query, target);
            EXPECT_EQ(rows.size(), matches ? 1 : 0);
            EXPECT_EQ(rows.contains(matching_row), matches);
        }
        return matches;
    };

    const auto fixed_rows = kr::dl::semantics::evaluate<Kind>(fixed, context);
    const auto goal_rows = kr::dl::semantics::evaluate<Kind>(goal, context);
    EXPECT_EQ(fixed_rows.size(), 2);
    EXPECT_EQ(goal_rows.size(), 1);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(fixed, context).get_storage_address(), fixed_rows.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(derived, context).size(), 4);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(ready, context).size(), 1);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(count, context).get(), 4);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(fixed_count, context).get(), 2);
    // A fully static join caches its result without retaining an additional index.
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(static_join, context).size(), fixed_rows.size());
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
        auto successor_context =
            kr::dl::semantics::StateEvaluationContext<Ext, Kind>(successor.get_state(), builder, storage, context.arguments(), context.registers());
        storage.reset_dynamic();
        empty_joins += !check_mixed_joins(successor_context);
        EXPECT_EQ(caches.get_static_join_indexes().size(), 1);
    }
    EXPECT_EQ(empty_joins, 1);
    auto next = kr::dl::semantics::StateEvaluationContext<Ext, Kind>(successors.front().get_state(),
                                                                     builder,
                                                                     storage,
                                                                     kr::dl::semantics::insert(denotations, empty_arguments).first,
                                                                     kr::dl::semantics::insert(denotations, registers).first);
    storage.reset_dynamic();
    EXPECT_TRUE(caches.get_queries(false).empty());
    EXPECT_TRUE(caches.template get<kr::dl::NumericalTag>(false).empty());
    EXPECT_TRUE(caches.get_queries(true).contains(fixed));
    EXPECT_TRUE(caches.template get<kr::dl::NumericalTag>(true).contains(fixed_count));
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(fixed, next).get_storage_address(), fixed_rows.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(goal, next).get_storage_address(), goal_rows.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(derived, next).size(), 3);
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(ready, next).empty());
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(count, next).get(), 3);

    const auto dynamic_role = kr::ps::ext::dl::parse_role(R"((r_project x z (q_atomic_state "triple" (x y z))))", domain, *repository);
    const auto static_role = kr::ps::ext::dl::parse_role(R"((r_atomic_state "edge"))", domain, *repository);
    const auto static_value = kr::dl::semantics::evaluate<Kind>(static_role, context);
    for (size_t i = 0; i < 16; ++i)
    {
        storage.reset_dynamic();
        EXPECT_EQ(storage.get_denotation_repository(false).template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
        EXPECT_EQ(static_value.count(), 2);
        const auto value = kr::dl::semantics::evaluate<Kind>(dynamic_role, i % 2 == 0 ? context : next);
        EXPECT_EQ(value.count(), i % 2 == 0 ? 4 : 3);
        EXPECT_EQ(storage.get_denotation_repository(false).template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 1);
        EXPECT_EQ(denotations.template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
    }

    // Materialize persistent results directly; source and target scratch storage remain independent.
    storage.reset_dynamic();
    auto persistent_memo = kr::dl::semantics::DenotationCaches<Ext> {};
    auto persistent = persistent_context(context, persistent_memo, denotations, storage);
    const auto retained = kr::dl::semantics::evaluate<Kind>(dynamic_role, persistent);
    EXPECT_EQ(storage.get_denotation_repository(false).template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
    EXPECT_FALSE(caches.template get<kr::dl::RoleTag>(false).contains(dynamic_role));
    const auto source_value = kr::dl::semantics::evaluate<Kind>(dynamic_role, context);
    auto target_storage = kr::dl::semantics::EvaluationStorage<Ext>(denotations);
    EXPECT_NE(&target_storage.get_denotation_repository(false), &storage.get_denotation_repository(false));
    EXPECT_NE(&storage.get_denotation_repository(false), &storage.get_denotation_repository(true));
    auto target_context =
        kr::dl::semantics::StateEvaluationContext<Ext, Kind>(successors.front().get_state(), builder, target_storage, next.arguments(), next.registers());
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(dynamic_role, target_context).count(), 3);
    EXPECT_NE(kr::dl::semantics::evaluate<Kind>(static_role, target_context), static_value);
    target_storage.reset_all();
    EXPECT_EQ(source_value.count(), 4);
    EXPECT_EQ(static_value.count(), 2);
    EXPECT_EQ(retained.count(), 4);

    storage.reset_all();
    EXPECT_EQ(caches.get_static_join_indexes().size(), 0);
    EXPECT_TRUE(caches.get_queries(true).empty());
    EXPECT_TRUE(caches.template get<kr::dl::NumericalTag>(true).empty());
    EXPECT_TRUE(caches.get_queries(false).empty());
    EXPECT_TRUE(caches.template get<kr::dl::NumericalTag>(false).empty());
    EXPECT_EQ(storage.get_denotation_repository(false).template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
    EXPECT_EQ(storage.get_denotation_repository(true).template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
    EXPECT_EQ(retained.count(), 4);
    // A full clear releases indexed rows as well; reevaluation must rebuild the shared index.
    EXPECT_TRUE(check_mixed_joins(context));
    EXPECT_EQ(caches.get_static_join_indexes().size(), 1);
    storage.reset_all();
    EXPECT_TRUE(caches.get_queries(false).empty());
    EXPECT_TRUE(caches.template get<kr::dl::NumericalTag>(false).empty());
}

template<tyr::TaskKind Kind>
void check_query_cache_across_bindings()
{
    const auto search = query_search_context<Kind>();
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage<Ext>(denotations);
    auto& caches = storage.get_caches();
    auto empty_arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = kr::dl::semantics::StateEvaluationContext<Ext, Kind>(state,
                                                                        builder,
                                                                        storage,
                                                                        kr::dl::semantics::insert(denotations, empty_arguments).first,
                                                                        kr::dl::semantics::insert(denotations, registers).first);
    const auto a = domain.get_constants()[0];
    const auto b = domain.get_constants()[1];
    const auto c = domain.get_constants()[2];
    auto persistent_memo = kr::dl::semantics::DenotationCaches<Ext> {};
    auto persistent = persistent_context(context, persistent_memo, denotations, storage);
    const auto a_set = kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_concept(R"((c_nominal "a"))", domain, *repository), persistent);
    const auto b_set = kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_concept(R"((c_nominal "b"))", domain, *repository), persistent);
    auto argument_values = ygg::Data<kr::dl::semantics::CallArguments>();
    argument_values.concept_arguments.push_back(a_set.get_index());
    const auto make_bound = [&]
    {
        return kr::dl::semantics::StateEvaluationContext<Ext, Kind>(state,
                                                                    builder,
                                                                    storage,
                                                                    kr::dl::semantics::insert(denotations, argument_values).first,
                                                                    kr::dl::semantics::insert(denotations, registers).first);
    };
    registers.concept_values[0] = a.get_index();
    registers.role_values[0] = ::cista::pair(a.get_index(), b.get_index());
    auto bound = make_bound();
    const auto fixed = parse_query(R"((q_atomic_state "fixed" (x y z)))", domain, *repository);
    const auto register_count =
        kr::ps::ext::dl::parse_numerical(R"((n_count (q_join (q_atomic_state "fixed" (x y z)) (q_concept x (c_register 0)))))", domain, *repository);
    const auto argument = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_concept x (c_argument 0))))", domain, *repository);
    const auto role_register = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_role (x y) (r_register 0))))", domain, *repository);
    const auto register_rhs = parse_query(R"((q_join (q_concept z (c_register 0)) (q_atomic_state "fixed" (x y z))))", domain, *repository);
    const auto role_register_rhs = parse_query(R"((q_join (q_role (x y) (r_register 0)) (q_atomic_state "fixed" (y x z))))", domain, *repository);

    const auto fixed_rows = kr::dl::semantics::evaluate<Kind>(fixed, bound);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(register_count, bound).get(), 1);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(argument, bound).size(), 1);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(role_register, bound).size(), 1);
    const auto rhs_rows = kr::dl::semantics::evaluate<Kind>(register_rhs, bound);
    EXPECT_EQ(rhs_rows.size(), 1);
    EXPECT_TRUE(rhs_rows.contains(std::tuple { a.get_index(), c.get_index(), b.get_index() }));
    EXPECT_TRUE(std::ranges::equal(rhs_rows.columns(), register_rhs.get_schema()));
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(role_register_rhs, bound).empty());
    registers.concept_values[0] = b.get_index();
    registers.role_values[0] = ::cista::pair(b.get_index(), c.get_index());
    argument_values.concept_arguments[0] = b_set.get_index();
    auto rebound = make_bound();
    storage.reset_dynamic();
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(fixed, rebound).get_storage_address(), fixed_rows.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(register_count, rebound).get(), 0);
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(argument, rebound).empty());
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(role_register, rebound).empty());
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(register_rhs, rebound).empty());
    // Reordered shared columns are matched by label, not by their position in either operand.
    const auto rhs_role_rows = kr::dl::semantics::evaluate<Kind>(role_register_rhs, rebound);
    EXPECT_EQ(rhs_role_rows.size(), 1);
    EXPECT_TRUE(rhs_role_rows.contains(std::tuple { b.get_index(), c.get_index(), a.get_index() }));
    EXPECT_TRUE(std::ranges::equal(rhs_role_rows.columns(), role_register_rhs.get_schema()));
    EXPECT_TRUE(caches.template get<kr::dl::NumericalTag>(false).contains(register_count));
    EXPECT_FALSE(caches.template get<kr::dl::NumericalTag>(true).contains(register_count));

    registers.concept_values[0].reset();
    registers.role_values[0].reset();
    argument_values.concept_arguments[0] = a_set.get_index();
    auto cleared = make_bound();
    storage.reset_dynamic();
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(register_count, cleared).get(), 0);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(argument, cleared).size(), 1);
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(role_register, cleared).empty());
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(register_rhs, cleared).empty());
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(role_register_rhs, cleared).empty());
    EXPECT_EQ(bound.registers().template get<kr::dl::ConceptTag>()[0].value().get_index(), a.get_index());
    EXPECT_EQ(bound.arguments().template get<kr::dl::ConceptTag>()[0].get_index(), a_set.get_index());
    EXPECT_EQ(rebound.registers().template get<kr::dl::ConceptTag>()[0].value().get_index(), b.get_index());
    EXPECT_EQ(rebound.arguments().template get<kr::dl::ConceptTag>()[0].get_index(), b_set.get_index());
}

}  // namespace

TEST(RunirQueries, GroundQueryCachePreservesStaticResultsAcrossStates) { check_query_cache_across_states<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedQueryCachePreservesStaticResultsAcrossStates) { check_query_cache_across_states<tyr::LiftedTag>(); }
TEST(RunirQueries, GroundQueryCacheInvalidatesRegistersAndArguments) { check_query_cache_across_bindings<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedQueryCacheInvalidatesRegistersAndArguments) { check_query_cache_across_bindings<tyr::LiftedTag>(); }
TEST(RunirQueries, CachedRenameSharesRowsWithoutChangingSchemasAndHandlesNullaryJoins)
{
    const auto search = query_search_context<tyr::GroundTag>();
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage<Ext>(denotations);
    auto& caches = storage.get_caches();
    auto empty_arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = kr::dl::semantics::StateEvaluationContext<Ext, tyr::GroundTag>(state,
                                                                                  builder,
                                                                                  storage,
                                                                                  kr::dl::semantics::insert(denotations, empty_arguments).first,
                                                                                  kr::dl::semantics::insert(denotations, registers).first);
    const auto description = std::string(R"((q_atomic_state "fixed" (x y z)))");
    const auto query = parse_query(description, domain, *repository);
    const auto renamed = parse_query("(q_rename (u v w) " + description + ")", domain, *repository);
    const auto nested = parse_query("(q_rename (p q r) (q_rename (u v w) " + description + "))", domain, *repository);
    const auto original_rows = kr::dl::semantics::evaluate<tyr::GroundTag>(query, context);
    const auto renamed_rows = kr::dl::semantics::evaluate<tyr::GroundTag>(renamed, context);
    const auto nested_rows = kr::dl::semantics::evaluate<tyr::GroundTag>(nested, context);
    EXPECT_EQ(renamed_rows.get_storage_address(), original_rows.get_storage_address());
    EXPECT_EQ(nested_rows.get_storage_address(), original_rows.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(renamed, context).get_storage_address(), original_rows.get_storage_address());
    EXPECT_TRUE(std::ranges::equal(original_rows.columns(), query.get_schema()));
    EXPECT_TRUE(std::ranges::equal(renamed_rows.columns(), renamed.get_schema()));
    EXPECT_TRUE(std::ranges::equal(nested_rows.columns(), nested.get_schema()));
    EXPECT_FALSE(std::ranges::equal(original_rows.columns(), renamed_rows.columns()));

    const auto truth = parse_query("(q_project () " + description + ")", domain, *repository);
    const auto true_rows = kr::dl::semantics::evaluate<tyr::GroundTag>(truth, context);
    EXPECT_TRUE(true_rows.columns().empty());
    EXPECT_EQ(true_rows.size(), 1);
    const auto true_join = parse_query("(q_join " + description + " (q_project () " + description + "))", domain, *repository);
    const auto false_join = parse_query("(q_join " + description + " (q_atomic_state \"missing\" ()))", domain, *repository);
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(true_join, context).size(), original_rows.size());
    EXPECT_TRUE(kr::dl::semantics::evaluate<tyr::GroundTag>(false_join, context).empty());
    EXPECT_TRUE(std::ranges::equal(kr::dl::semantics::evaluate<tyr::GroundTag>(query, context).columns(), query.get_schema()));

    auto raw_rename_data = ygg::Data<kr::dl::Query<Ext, kr::dl::QueryRenameTag>>();
    raw_rename_data.arg = query.get_index();
    for (const auto column : query.get_columns())
        raw_rename_data.columns.push_back(column.get_index());
    std::swap(raw_rename_data.columns.front(), raw_rename_data.columns.back());
    auto raw_wrapper = ygg::Data<kr::dl::Query<Ext>>(kr::dl::insert(*repository, raw_rename_data).first.get_index());
    const auto raw = repository->insert(raw_wrapper).first;
    EXPECT_FALSE(raw.is_static());
    const auto raw_rows = kr::dl::semantics::evaluate<tyr::GroundTag>(raw, context);
    EXPECT_EQ(raw_rows.get_storage_address(), original_rows.get_storage_address());
    EXPECT_TRUE(caches.get_queries(false).contains(raw));

    auto raw_nested_data = ygg::Data<kr::dl::Query<Ext, kr::dl::QueryRenameTag>>();
    raw_nested_data.arg = raw.get_index();
    for (const auto column : nested.get_columns())
        raw_nested_data.columns.push_back(column.get_index());
    auto raw_nested_wrapper = ygg::Data<kr::dl::Query<Ext>>(kr::dl::insert(*repository, raw_nested_data).first.get_index());
    const auto raw_nested = repository->insert(raw_nested_wrapper).first;
    EXPECT_FALSE(raw_nested.is_static());
    const auto raw_nested_rows = kr::dl::semantics::evaluate<tyr::GroundTag>(raw_nested, context);
    EXPECT_EQ(raw_nested_rows.get_storage_address(), original_rows.get_storage_address());
    EXPECT_TRUE(std::ranges::equal(raw_nested_rows.columns(), raw_nested.get_schema()));
    EXPECT_TRUE(std::ranges::equal(raw_rows.columns(), raw.get_schema()));
    EXPECT_TRUE(std::ranges::equal(original_rows.columns(), query.get_schema()));
    EXPECT_TRUE(caches.get_queries(false).contains(raw_nested));

    storage.reset_all();
    EXPECT_FALSE(caches.get_queries(false).contains(raw));
    EXPECT_FALSE(caches.get_queries(false).contains(raw_nested));
    EXPECT_FALSE(caches.get_queries(true).contains(query));
    const auto rebuilt_nested = kr::dl::semantics::evaluate<tyr::GroundTag>(raw_nested, context);
    EXPECT_EQ(rebuilt_nested.size(), 2);
    EXPECT_TRUE(std::ranges::equal(rebuilt_nested.columns(), raw_nested.get_schema()));
    EXPECT_EQ(rebuilt_nested.get_storage_address(), kr::dl::semantics::evaluate<tyr::GroundTag>(query, context).get_storage_address());
    EXPECT_TRUE(caches.get_queries(false).contains(raw_nested));
    const auto rebuilt = kr::dl::semantics::evaluate<tyr::GroundTag>(raw, context);
    EXPECT_EQ(rebuilt.size(), 2);
    EXPECT_TRUE(std::ranges::equal(rebuilt.columns(), raw.get_schema()));
    EXPECT_EQ(rebuilt.get_storage_address(), kr::dl::semantics::evaluate<tyr::GroundTag>(query, context).get_storage_address());
    EXPECT_TRUE(caches.get_queries(false).contains(raw));
}

}  // namespace runir::tests
