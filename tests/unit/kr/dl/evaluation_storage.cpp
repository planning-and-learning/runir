#include "planning_fixtures.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <filesystem>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/dl/semantics/interning.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <tyr/planning/ground/successor_generator.hpp>
#include <utility>

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

TEST(RunirEvaluationStorage, DenotationBuildersInternValuesAndUpdateTheirIndices)
{
    const auto search = query_search();
    auto repository = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto builder = sem::Builder();
    const auto check = [&]<dl::CategoryTag Category>()
    {
        using Denotation = sem::Denotation<Category>;
        auto source = ygg::Builder<Denotation>();
        if constexpr (dl::ConceptOrRoleTag<Category>)
        {
            source.initialize(3);
            if constexpr (std::same_as<Category, dl::ConceptTag>)
                source.get().set(1);
            else
                source.get(0).set(2);
        }
        else if constexpr (std::same_as<Category, dl::BooleanTag>)
            source.get() = true;
        else
            source.get() = 7;
        const auto expected = source;
        const auto [view, inserted] = sem::get_or_create(repository, source, builder);
        EXPECT_TRUE(inserted);
        EXPECT_EQ(&view.get_context(), &repository);
        EXPECT_EQ(source.index, view.get_index());
        EXPECT_TRUE(ygg::EqualTo<ygg::Builder<Denotation>> {}(source, expected));

        ygg::clear(source.index);
        const auto [duplicate, duplicate_inserted] = sem::get_or_create(repository, source, builder);
        EXPECT_FALSE(duplicate_inserted);
        EXPECT_EQ(duplicate, view);
        EXPECT_EQ(source.index, view.get_index());
        EXPECT_EQ(repository.size<Denotation>(), 1);

        source.initialize(0);
        if constexpr (std::same_as<Category, dl::ConceptTag>)
        {
            EXPECT_EQ(view.get().count(), 1);
            EXPECT_TRUE(view.get().test(1));
        }
        else if constexpr (std::same_as<Category, dl::RoleTag>)
        {
            EXPECT_EQ(view.get_num_objects(), 3);
            EXPECT_EQ(view.count(), 1);
            EXPECT_TRUE(view.get(0).test(2));
        }
        else
            EXPECT_EQ(view.get(), expected.get());
    };
    check.template operator()<dl::BooleanTag>();
    check.template operator()<dl::NumericalTag>();
    check.template operator()<dl::ConceptTag>();
    check.template operator()<dl::RoleTag>();
}

TEST(RunirEvaluationStorage, RegisterInterningUsesRepositoryOwnershipAndPreservesBorrowedValues)
{
    const auto search = query_search();
    const auto formalism = search->task->get_repository();
    const auto objects = search->task->get_domain().get_domain().get_constants();
    auto repository = sem::DenotationRepositoryFactory().create(formalism);
    auto other_repository = sem::DenotationRepositoryFactory().create(formalism);
    auto builder = sem::Builder();
    auto values = ygg::Data<sem::RegisterValues>();
    values.index = ygg::Index<sem::RegisterValues>(42);
    values.concept_values.resize(2);
    values.concept_values[0] = objects[0].get_index();
    values.role_values.emplace_back(::cista::pair { objects[1].get_index(), objects[2].get_index() });
    const auto expected = values;
    const auto borrowed = ygg::make_view(values, *formalism);
    const auto [view, inserted] = sem::get_or_create(repository, borrowed, builder);
    EXPECT_TRUE(inserted);
    EXPECT_EQ(&view.get_context(), &repository);
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<sem::RegisterValues>> {}(view.get_data(), expected));
    const auto [duplicate, duplicate_inserted] = sem::get_or_create(repository, borrowed, builder);
    EXPECT_FALSE(duplicate_inserted);
    EXPECT_EQ(duplicate, view);
    const auto [same_repository, same_inserted] = sem::get_or_create(repository, view, builder);
    EXPECT_FALSE(same_inserted);
    EXPECT_EQ(&same_repository.get_data(), &view.get_data());
    EXPECT_EQ(repository.size<sem::RegisterValues>(), 1);
    EXPECT_EQ(values.index, expected.index);
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<sem::RegisterValues>> {}(values, expected));

    auto other_values = expected;
    other_values.concept_values[0] = objects[1].get_index();
    const auto occupied = sem::get_or_create(other_repository, other_values).first;
    ASSERT_EQ(other_repository.get_index(), repository.get_index());
    ASSERT_EQ(occupied.get_index(), view.get_index());
    const auto [transferred, transferred_inserted] = sem::get_or_create(other_repository, view, builder);
    EXPECT_TRUE(transferred_inserted);
    EXPECT_EQ(&transferred.get_context(), &other_repository);
    EXPECT_NE(transferred.get_index(), occupied.get_index());
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<sem::RegisterValues>> {}(transferred.get_data(), expected));
    const auto [same_target, same_target_inserted] = sem::get_or_create(other_repository, view, builder);
    EXPECT_EQ(same_target, transferred);
    EXPECT_FALSE(same_target_inserted);
    EXPECT_EQ(other_repository.size<sem::RegisterValues>(), 2);

    const auto foreign_search = query_search();
    auto foreign_repository = sem::DenotationRepositoryFactory().create(foreign_search->task->get_repository());
    EXPECT_THROW((void) sem::get_or_create(foreign_repository, borrowed, builder), std::invalid_argument);
    EXPECT_EQ(foreign_repository.size<sem::RegisterValues>(), 0);
    auto foreign_values = expected;
    const auto foreign_view = sem::get_or_create(foreign_repository, foreign_values).first;
    EXPECT_THROW((void) sem::get_or_create(repository, foreign_view, builder), std::invalid_argument);
    EXPECT_EQ(repository.size<sem::RegisterValues>(), 1);
}

TEST(RunirEvaluationStorage, RegisterExtractionClearsIdentityAndRetainsBuffers)
{
    const auto search = query_search();
    const auto formalism = search->task->get_repository();
    const auto objects = search->task->get_domain().get_domain().get_constants();
    auto repository = sem::DenotationRepositoryFactory().create(formalism);
    auto values = ygg::Data<sem::RegisterValues>();
    values.concept_values.resize(2);
    values.concept_values[0] = objects[0].get_index();
    values.role_values.emplace_back(::cista::pair { objects[1].get_index(), objects[2].get_index() });
    const auto view = sem::get_or_create(repository, values).first;
    auto extracted = ygg::Data<sem::RegisterValues>();
    extracted.index = view.get_index();
    sem::make_data(view, extracted);
    EXPECT_EQ(extracted.index, ygg::Index<sem::RegisterValues>());
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<sem::RegisterValues>> {}(extracted, values));
    const auto concept_buffer = extracted.concept_values.data();
    const auto concept_capacity = extracted.concept_values.allocated_size_;
    const auto role_buffer = extracted.role_values.data();
    const auto role_capacity = extracted.role_values.allocated_size_;

    values.concept_values.resize(1);
    values.concept_values[0] = objects[2].get_index();
    values.role_values[0].reset();
    extracted.index = view.get_index();
    sem::make_data(ygg::make_view(values, *formalism), extracted);
    EXPECT_EQ(extracted.index, ygg::Index<sem::RegisterValues>());
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<sem::RegisterValues>> {}(extracted, values));
    EXPECT_EQ(extracted.concept_values.data(), concept_buffer);
    EXPECT_EQ(extracted.concept_values.allocated_size_, concept_capacity);
    EXPECT_EQ(extracted.role_values.data(), role_buffer);
    EXPECT_EQ(extracted.role_values.allocated_size_, role_capacity);
    EXPECT_EQ(view.get_data().concept_values.size(), 2);
    EXPECT_EQ(view.get_data().concept_values[0].value(), objects[0].get_index());
    EXPECT_TRUE(view.get_data().role_values[0].has_value());
}

TEST(RunirEvaluationStorage, TypedRegisterAssignmentPreservesBuffersAndClearsIdentity)
{
    const auto search = query_search();
    const auto objects = search->task->get_domain().get_domain().get_constants();
    auto values = ygg::Data<sem::RegisterValues>();
    values.concept_values.resize(2);
    values.role_values.resize(2);
    values.concept_values[0] = objects[0].get_index();
    const auto concept_buffer = values.concept_values.data();
    const auto role_buffer = values.role_values.data();

    values.index = ygg::Index<sem::RegisterValues>(42);
    sem::assign_register(values, dl::RegisterIdentifier<dl::ConceptTag>(1), objects[1]);
    EXPECT_EQ(values.index, ygg::Index<sem::RegisterValues>());
    EXPECT_EQ(values.concept_values[0].value(), objects[0].get_index());
    EXPECT_EQ(values.concept_values[1].value(), objects[1].get_index());

    values.index = ygg::Index<sem::RegisterValues>(42);
    sem::assign_register(values, dl::RegisterIdentifier<dl::RoleTag>(1), std::pair(objects[1], objects[2]));
    EXPECT_EQ(values.index, ygg::Index<sem::RegisterValues>());
    EXPECT_FALSE(values.role_values[0].has_value());
    EXPECT_EQ(values.role_values[1].value().first, objects[1].get_index());
    EXPECT_EQ(values.role_values[1].value().second, objects[2].get_index());
    EXPECT_EQ(values.concept_values.data(), concept_buffer);
    EXPECT_EQ(values.role_values.data(), role_buffer);

    values.index = ygg::Index<sem::RegisterValues>(42);
    const auto expected = values;
    EXPECT_THROW(sem::assign_register(values, dl::RegisterIdentifier<dl::ConceptTag>(2), objects[2]), std::out_of_range);
    EXPECT_THROW(sem::assign_register(values, dl::RegisterIdentifier<dl::RoleTag>(2), std::pair(objects[2], objects[0])), std::out_of_range);
    EXPECT_EQ(values.index, expected.index);
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<sem::RegisterValues>> {}(values, expected));
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

TEST(RunirEvaluationStorage, ExtContextsValidateInputTasksForBorrowedAndIndexedRegisters)
{
    const auto search = query_search();
    const auto foreign_search = query_search();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto builder = sem::Builder();
    auto repository = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto input_repository = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto foreign_repository = sem::DenotationRepositoryFactory().create(foreign_search->task->get_repository());
    auto storage = sem::EvaluationStorage<Ext>(repository);
    auto caches = sem::DenotationCaches<Ext>();
    auto arguments_data = ygg::Data<sem::CallArguments>();
    auto registers_data = ygg::Data<sem::RegisterValues>();
    const auto arguments = sem::get_or_create(input_repository, arguments_data).first;
    const auto foreign_arguments = sem::get_or_create(foreign_repository, arguments_data).first;
    const auto check = [&](auto registers, auto foreign_registers)
    {
        using Context = sem::StateEvaluationContext<Ext, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, decltype(registers)>;
        EXPECT_NO_THROW(Context(initial.get_state(), builder, storage, arguments, registers));
        EXPECT_NO_THROW(Context(initial.get_state(), builder, caches, repository, storage, arguments, registers));
        EXPECT_THROW(Context(initial.get_state(), builder, storage, foreign_arguments, registers), std::invalid_argument);
        EXPECT_THROW(Context(initial.get_state(), builder, caches, repository, storage, foreign_arguments, registers), std::invalid_argument);
        EXPECT_THROW(Context(initial.get_state(), builder, storage, arguments, foreign_registers), std::invalid_argument);
        EXPECT_THROW(Context(initial.get_state(), builder, caches, repository, storage, arguments, foreign_registers), std::invalid_argument);
    };
    check(ygg::make_view(registers_data, *search->task->get_repository()), ygg::make_view(registers_data, *foreign_search->task->get_repository()));
    check(sem::get_or_create(input_repository, registers_data).first, sem::get_or_create(foreign_repository, registers_data).first);
}

TEST(RunirEvaluationStorage, QueryResultsInternByOrderedNumericSchemaAndRows)
{
    const auto search = query_search();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto factory = dl::ConstructorRepositoryFactoryFor<Ext>();
    auto first_repository = factory.create(search->task->get_repository());
    auto second_repository = factory.create(search->task->get_repository());
    auto builder = sem::Builder();
    auto persistent = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<Ext>(persistent);
    auto caches = sem::DenotationCaches<Ext>();
    auto arguments_data = ygg::Data<sem::CallArguments>();
    auto registers_data = ygg::Data<sem::RegisterValues>();
    auto context = sem::StateEvaluationContext<Ext, tyr::GroundTag>(initial.get_state(),
                                                                    builder,
                                                                    caches,
                                                                    persistent,
                                                                    storage,
                                                                    sem::get_or_create(persistent, arguments_data).first,
                                                                    sem::get_or_create(persistent, registers_data).first);
    const auto query = [&](const std::string& expression, auto& repository)
    {
        const auto count = kr::ps::ext::dl::parse_numerical("(n_count " + expression + ")", search->task->get_domain().get_domain(), repository);
        return count.get_variant().template get<ygg::Index<dl::Numerical<Ext, dl::CountTag>>>().get_arg().template get<ygg::Index<dl::Query<Ext>>>();
    };
    const auto left = query("(q_role (left_source left_target) (r_universal))", *first_repository);
    const auto right = query("(q_role (right_source right_target) (r_universal))", *second_repository);
    ASSERT_EQ(left.get_columns()[0].get_index(), right.get_columns()[0].get_index());
    ASSERT_NE(left.get_columns()[0].get_name(), right.get_columns()[0].get_name());
    using ColumnIndex = ygg::Index<ygg::database::Column>;
    const auto columns = std::array { ColumnIndex(0), ColumnIndex(1) };
    const auto reversed_columns = std::array { ColumnIndex(1), ColumnIndex(0) };
    auto& results = persistent.get_relation_repository();
    const auto left_result = sem::evaluate(left, context);
    const auto right_result = sem::evaluate(right, context);
    ASSERT_EQ(left_result.size(), 9);
    EXPECT_TRUE(std::ranges::equal(left_result.columns(), columns));
    EXPECT_EQ(&left_result.get_context(), &results);
    EXPECT_EQ(left_result, right_result);
    EXPECT_EQ(left_result.get_storage_address(), right_result.get_storage_address());
    EXPECT_EQ(results.size(), 1);

    const auto left_alias = query("(q_rename (left_target left_source) (q_role (left_source left_target) (r_universal)))", *first_repository);
    const auto right_alias = query("(q_rename (right_target right_source) (q_role (right_source right_target) (r_universal)))", *second_repository);
    ASSERT_EQ(left_alias.get_columns()[0].get_index(), right_alias.get_columns()[0].get_index());
    const auto renamed_left = sem::evaluate(left_alias, context);
    const auto renamed_right = sem::evaluate(right_alias, context);
    EXPECT_TRUE(std::ranges::equal(renamed_left.columns(), reversed_columns));
    EXPECT_EQ(renamed_left, renamed_right);
    EXPECT_NE(renamed_left, left_result);
    EXPECT_EQ(renamed_left.get_storage_address(), left_result.get_storage_address());
    EXPECT_EQ(renamed_right.get_storage_address(), right_result.get_storage_address());
    EXPECT_EQ(results.size(), 2);

    {
        auto direct = builder.get_builder<ygg::database::Relation<>>(columns);
        for (size_t i = 0; i < left_result.size(); ++i)
            direct->insert(left_result[i]);
        const auto [interned, inserted] = ygg::database::intern_relation(*direct, results);
        EXPECT_FALSE(inserted);
        EXPECT_EQ(interned, left_result);
    }

    caches.reset_all();
    storage.reset_all();
    first_repository->clear();
    first_repository.reset();
    second_repository.reset();
    EXPECT_EQ(left_result.size(), 9);
    EXPECT_TRUE(left_result.contains({ 0, 2 }));
    EXPECT_TRUE(std::ranges::equal(renamed_left.columns(), reversed_columns));
    auto fresh_repository = factory.create(search->task->get_repository());
    const auto fresh = query("(q_role (fresh_source fresh_target) (r_universal))", *fresh_repository);
    EXPECT_EQ(sem::evaluate(fresh, context), left_result);
    const auto fresh_alias = query("(q_rename (fresh_target fresh_source) (q_role (fresh_source fresh_target) (r_universal)))", *fresh_repository);
    EXPECT_EQ(sem::evaluate(fresh_alias, context), renamed_left);
    EXPECT_EQ(results.size(), 2);
}

}  // namespace
}  // namespace runir::tests
