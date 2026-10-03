#include "planning_fixtures.hpp"

#include <array>
#include <concepts>
#include <filesystem>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <tyr/planning/ground/successor_generator.hpp>

namespace runir::tests
{
namespace
{
namespace dl = kr::dl;
namespace sem = dl::semantics;
using Ext = kr::ExtFamilyTag;

auto query_search()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
}

TEST(RunirEvaluationStorage, DurableRootsDoNotReuseTransientRootEntries)
{
    const auto search = query_search();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto repository = dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = sem::Builder();
    auto persistent = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<Ext>(persistent);
    auto root_caches = sem::DenotationCaches<Ext>();
    auto arguments_data = ygg::Data<sem::CallArguments>();
    auto registers_data = ygg::Data<sem::RegisterValues>();
    registers_data.concept_values.resize(1);
    registers_data.concept_values[0] = search->task->get_domain().get_domain().get_constants()[0].get_index();
    const auto arguments = sem::get_or_create(persistent, arguments_data).first;
    const auto registers = ygg::make_view(registers_data, *search->task->get_repository());
    using Context = sem::StateEvaluationContext<Ext, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, sem::BorrowedRegisterValuesView>;
    auto transient = Context(initial.get_state(), builder, storage, arguments, registers);
    auto durable = Context(initial.get_state(), builder, root_caches, persistent, storage, arguments, registers);
    const auto expression =
        kr::ps::ext::dl::parse_role(R"((r_project x z (q_atomic_state "triple" (x y z))))", search->task->get_domain().get_domain(), *repository);

    static_assert(std::same_as<decltype(durable.for_result(false)), Context>);
    static_assert(std::same_as<decltype(durable.child_context()), Context>);
    for (const auto is_static : { false, true })
    {
        auto transient_result = transient.for_result(is_static);
        auto durable_result = durable.for_result(is_static);
        auto transient_children = transient.child_context().for_result(is_static);
        auto durable_children = durable.child_context().for_result(is_static);
        EXPECT_EQ(&durable_result.get_denotation_repository(), &persistent);
        EXPECT_EQ(&durable_result.get_caches(), &root_caches);
        for (auto scratch_context : { transient_result, transient_children, durable_children })
        {
            EXPECT_EQ(&scratch_context.get_denotation_repository(), &storage.get_denotation_repository(is_static));
            EXPECT_EQ(&scratch_context.get_caches(), &storage.get_caches());
        }
        for (auto copy : { transient_result, durable_result, transient_children, durable_children })
        {
            EXPECT_EQ(&copy.get_workspace(), &builder.get_workspace());
            EXPECT_EQ(&copy.arguments().get_data(), &arguments.get_data());
            EXPECT_EQ(&copy.registers().get_data(), &registers_data);
            EXPECT_EQ(copy.registers().at(dl::RegisterIdentifier<dl::ConceptTag>(0)).value().get_index(), registers_data.concept_values[0].value());
        }
    }
    EXPECT_EQ(&transient.get_denotation_repository(), &storage.get_denotation_repository(false));
    EXPECT_EQ(&transient.get_caches(), &storage.get_caches());
    EXPECT_EQ(&durable.get_denotation_repository(), &persistent);
    EXPECT_EQ(&durable.get_caches(), &root_caches);
    EXPECT_EQ(&transient.get_workspace(), &builder.get_workspace());
    EXPECT_EQ(&durable.get_workspace(), &builder.get_workspace());

    const auto temporary = sem::evaluate(expression, transient);
    EXPECT_EQ(&temporary.get_context(), &storage.get_denotation_repository(false));
    EXPECT_EQ(persistent.size<sem::Denotation<dl::RoleTag>>(), 0);
    const auto retained = sem::evaluate(expression, durable);
    EXPECT_EQ(&retained.get_context(), &persistent);
    EXPECT_NE(retained, temporary);
    EXPECT_EQ(retained.count(), temporary.count());
    EXPECT_EQ(sem::evaluate(expression, durable), retained);
    EXPECT_TRUE(root_caches.get<dl::RoleTag>(false).contains(expression));

    root_caches.reset_dynamic();
    storage.reset_dynamic();
    EXPECT_TRUE(root_caches.get<dl::RoleTag>(false).empty());
    EXPECT_EQ(retained.count(), 4);
    EXPECT_EQ(sem::evaluate(expression, durable), retained);
    EXPECT_EQ(persistent.size<sem::Denotation<dl::RoleTag>>(), 1);
    EXPECT_EQ(storage.get_denotation_repository(false).size<sem::Denotation<dl::RoleTag>>(), 0);
}

TEST(RunirEvaluationStorage, DynamicResetPreservesStaticRowsAndJoinIndexes)
{
    const auto search = query_search();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto constructors = dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = sem::Builder();
    auto prototype = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<Ext>(prototype);
    auto arguments = ygg::Data<sem::CallArguments>();
    auto registers = ygg::Data<sem::RegisterValues>();
    auto context = sem::StateEvaluationContext<Ext, tyr::GroundTag>(initial.get_state(),
                                                                    builder,
                                                                    storage,
                                                                    sem::get_or_create(prototype, arguments).first,
                                                                    sem::get_or_create(prototype, registers).first);
    const auto query = [&](const std::string& expression)
    {
        const auto count = kr::ps::ext::dl::parse_numerical("(n_count " + expression + ")", domain, *constructors);
        return count.get_variant().template get<ygg::Index<dl::Numerical<Ext, dl::CountTag>>>().get_arg().template get<ygg::Index<dl::Query<Ext>>>();
    };
    const auto fixed_query = query(R"((q_atomic_state "fixed" (x y z)))");
    const auto renamed_query = query(R"((q_rename (a b c) (q_atomic_state "fixed" (x y z))))");
    const auto dynamic_query = query(R"((q_atomic_state "triple" (x y z)))");
    const auto fixed = sem::evaluate(fixed_query, context);
    const auto alias = sem::evaluate(renamed_query, context);
    EXPECT_EQ(sem::evaluate(dynamic_query, context).size(), 4);
    const auto keys = std::array<size_t, 1> { 0 };
    storage.get_caches().get_static_join_indexes().get_or_create(fixed, keys);
    EXPECT_EQ(alias.get_storage_address(), fixed.get_storage_address());
    EXPECT_NE(alias, fixed);

    storage.reset_dynamic();
    EXPECT_TRUE(storage.get_denotation_repository(false).get_relation_repository().empty());
    EXPECT_EQ(fixed.size(), 2);
    EXPECT_EQ(storage.get_caches().get_static_join_indexes().size(), 1);
    EXPECT_EQ(sem::evaluate(fixed_query, context), fixed);
    EXPECT_EQ(sem::evaluate(renamed_query, context), alias);
    EXPECT_EQ(sem::evaluate(dynamic_query, context).size(), 4);

    storage.reset_all();
    EXPECT_TRUE(storage.get_denotation_repository(false).get_relation_repository().empty());
    EXPECT_TRUE(storage.get_denotation_repository(true).get_relation_repository().empty());
    EXPECT_EQ(storage.get_caches().get_static_join_indexes().size(), 0);
    const auto rebuilt = sem::evaluate(fixed_query, context);
    EXPECT_EQ(rebuilt.size(), 2);
    EXPECT_EQ(sem::evaluate(renamed_query, context).get_storage_address(), rebuilt.get_storage_address());
}

TEST(RunirEvaluationStorage, PreparedContextsRejectForeignTaskRepositories)
{
    const auto search = query_search();
    const auto foreign_search = query_search();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto builder = sem::Builder();
    auto prototype = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto foreign_prototype = sem::DenotationRepositoryFactory().create(foreign_search->task->get_repository());
    auto storage = sem::EvaluationStorage<kr::BaseFamilyTag>(prototype);
    auto foreign_storage = sem::EvaluationStorage<kr::BaseFamilyTag>(foreign_prototype);
    using Context = sem::StateEvaluationContext<kr::BaseFamilyTag, tyr::GroundTag>;
    auto caches = sem::DenotationCaches<kr::BaseFamilyTag>();
    EXPECT_NO_THROW(Context(initial.get_state(), builder, storage));
    EXPECT_NO_THROW(Context(initial.get_state(), builder, caches, prototype, storage));
    EXPECT_THROW(Context(initial.get_state(), builder, foreign_storage), std::invalid_argument);
    EXPECT_THROW(Context(initial.get_state(), builder, caches, foreign_prototype, storage), std::invalid_argument);
    EXPECT_THROW(Context(initial.get_state(), builder, caches, prototype, foreign_storage), std::invalid_argument);
}

TEST(RunirEvaluationStorage, QuerySchemasKeepTheirConstructorRepositoryNamespace)
{
    const auto search = query_search();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto factory = dl::ConstructorRepositoryFactoryFor<Ext>();
    auto first_repository = factory.create(search->task->get_repository());
    auto second_repository = factory.create(search->task->get_repository());
    auto builder = sem::Builder();
    auto persistent = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<Ext>(persistent);
    auto arguments_data = ygg::Data<sem::CallArguments>();
    auto registers_data = ygg::Data<sem::RegisterValues>();
    auto context = sem::StateEvaluationContext<Ext, tyr::GroundTag>(initial.get_state(),
                                                                    builder,
                                                                    storage,
                                                                    sem::get_or_create(persistent, arguments_data).first,
                                                                    sem::get_or_create(persistent, registers_data).first);
    const auto query = [&](const std::string& expression, auto& repository)
    {
        const auto count = kr::ps::ext::dl::parse_numerical("(n_count " + expression + ")", search->task->get_domain().get_domain(), repository);
        return count.get_variant().template get<ygg::Index<dl::Numerical<Ext, dl::CountTag>>>().get_arg().template get<ygg::Index<dl::Query<Ext>>>();
    };
    const auto left = query("(q_concept left (c_top))", *first_repository);
    const auto right = query("(q_concept right (c_top))", *second_repository);
    ASSERT_EQ(left.get_columns()[0].get_index(), right.get_columns()[0].get_index());
    ASSERT_NE(left.get_columns()[0].get_name(), right.get_columns()[0].get_name());
    const auto left_result = sem::evaluate(left, context);
    const auto right_result = sem::evaluate(right, context);
    EXPECT_EQ(left_result.size(), right_result.size());
    EXPECT_EQ(left_result.columns()[0], right_result.columns()[0]);
    EXPECT_NE(left_result, right_result);

    const auto left_alias = query("(q_rename (renamed_left) (q_concept left (c_top)))", *first_repository);
    const auto right_alias = query("(q_rename (renamed_right) (q_concept right (c_top)))", *second_repository);
    ASSERT_EQ(left_alias.get_columns()[0].get_index(), right_alias.get_columns()[0].get_index());
    const auto renamed_left = sem::evaluate(left_alias, context);
    const auto renamed_right = sem::evaluate(right_alias, context);
    EXPECT_NE(renamed_left, renamed_right);
    EXPECT_EQ(renamed_left.get_storage_address(), left_result.get_storage_address());
    EXPECT_EQ(renamed_right.get_storage_address(), right_result.get_storage_address());
}

}  // namespace
}  // namespace runir::tests
