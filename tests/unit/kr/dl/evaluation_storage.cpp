#include "planning_fixtures.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <filesystem>
#include <gtest/gtest.h>
#include <ranges>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_view.hpp>
#include <runir/kr/dl/semantics/detail/evaluation_impl.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/dl/semantics/interning.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>
#include <tyr/formalism/object_data.hpp>
#include <tyr/planning/ground/successor_generator.hpp>
#include <utility>

namespace runir::tests
{
namespace
{
using Ext = kr::ExtFamilyTag;
using ObjectIndex = ygg::Index<tyr::formalism::Object>;
using QueryValues = kr::dl::QueryValues;

using GroundExtContext = kr::dl::semantics::StateEvaluationContext<Ext, tyr::GroundTag>;
struct InvalidResultContext : GroundExtContext
{
    int for_result(bool) const;
};
struct MissingExtInputs : GroundExtContext
{
    kr::dl::semantics::BaseStateEvaluationContext<Ext, tyr::GroundTag> child_context() const;
};
struct WrongFamilyContext : GroundExtContext
{
    kr::dl::semantics::StateEvaluationContext<kr::BaseFamilyTag, tyr::GroundTag> child_context() const;
};
struct WrongKindContext : GroundExtContext
{
    kr::dl::semantics::StateEvaluationContext<Ext, tyr::LiftedTag> child_context() const;
};
static_assert(kr::dl::semantics::StateEvaluationContextConcept<GroundExtContext, Ext, tyr::GroundTag>);
static_assert(kr::dl::semantics::StateEvaluationContextConcept<kr::dl::semantics::StateEvaluationContext<kr::BaseFamilyTag, tyr::GroundTag>,
                                                               kr::BaseFamilyTag,
                                                               tyr::GroundTag>);
static_assert(kr::dl::semantics::
                  StateEvaluationContextConcept<kr::dl::semantics::StateEvaluationContext<kr::UnsFamilyTag, tyr::GroundTag>, kr::UnsFamilyTag, tyr::GroundTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<InvalidResultContext, Ext, tyr::GroundTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<MissingExtInputs, Ext, tyr::GroundTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<WrongFamilyContext, Ext, tyr::GroundTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<WrongKindContext, Ext, tyr::GroundTag>);

constexpr auto acquires_builder = []<typename T, typename... Args>()
{ return requires(kr::dl::semantics::Builder& builder, Args&&... args) { builder.template get_builder<T>(std::forward<Args>(args)...); }; };
static_assert(acquires_builder.template operator()<kr::dl::semantics::Denotation<kr::dl::ConceptTag>, ygg::uint_t>());
static_assert(acquires_builder.template operator()<ygg::database::Relation<QueryValues>, std::span<const ygg::Index<ygg::database::Column>>>());
static_assert(!acquires_builder.template operator()<int>());
static_assert(!acquires_builder.template operator()<kr::dl::semantics::Denotation<kr::dl::ConceptTag>, std::string>());

auto query_search()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
}

TEST(RunirEvaluationStorage, DenotationBuildersInternValuesAndUpdateTheirIndices)
{
    const auto search = query_search();
    auto repository = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    const auto check = [&]<kr::dl::CategoryTag Category>()
    {
        using Denotation = kr::dl::semantics::Denotation<Category>;
        auto source = ygg::Builder<Denotation>();
        if constexpr (kr::dl::ConceptOrRoleTag<Category>)
        {
            source.initialize(3);
            if constexpr (std::same_as<Category, kr::dl::ConceptTag>)
                source.get().set(1);
            else
                source.get(0).set(2);
        }
        else if constexpr (std::same_as<Category, kr::dl::BooleanTag>)
            source.get() = true;
        else
            source.get() = 7;
        const auto expected = source;
        const auto [view, inserted] = kr::dl::semantics::insert(repository, source, builder);
        EXPECT_TRUE(inserted);
        EXPECT_EQ(&view.get_context(), &repository);
        EXPECT_EQ(source.index, view.get_index());
        EXPECT_TRUE(ygg::EqualTo<ygg::Builder<Denotation>> {}(source, expected));

        ygg::clear(source.index);
        const auto [duplicate, duplicate_inserted] = kr::dl::semantics::insert(repository, source, builder);
        EXPECT_FALSE(duplicate_inserted);
        EXPECT_EQ(duplicate, view);
        EXPECT_EQ(source.index, view.get_index());
        EXPECT_EQ(repository.size<Denotation>(), 1);

        source.initialize(0);
        if constexpr (std::same_as<Category, kr::dl::ConceptTag>)
        {
            EXPECT_EQ(view.get().count(), 1);
            EXPECT_TRUE(view.get().test(1));
        }
        else if constexpr (std::same_as<Category, kr::dl::RoleTag>)
        {
            EXPECT_EQ(view.get_num_objects(), 3);
            EXPECT_EQ(view.count(), 1);
            EXPECT_TRUE(view.get(0).test(2));
        }
        else
            EXPECT_EQ(view.get(), expected.get());
    };
    check.template operator()<kr::dl::BooleanTag>();
    check.template operator()<kr::dl::NumericalTag>();
    check.template operator()<kr::dl::ConceptTag>();
    check.template operator()<kr::dl::RoleTag>();
}

TEST(RunirEvaluationStorage, BorrowedDenotationsExposeReadOnlyValuesAndMatchIndexedIteration)
{
    constexpr auto digits = static_cast<ygg::uint_t>(ygg::BitsetSpan<const ygg::uint_t>::Digits);
    constexpr auto num_objects = digits + 3;
    auto formalism = tyr::formalism::planning::RepositoryFactory().create_shared();
    for (ygg::uint_t i = 0; i < num_objects; ++i)
    {
        auto object = ygg::Data<tyr::formalism::Object>("object" + std::to_string(i));
        (void) formalism->insert(object);
    }
    auto repository = kr::dl::semantics::DenotationRepositoryFactory().create(formalism);
    auto scratch = kr::dl::semantics::Builder();
    const auto object = [&](ygg::uint_t index) { return ygg::make_view(ObjectIndex(index), *formalism); };

    auto boolean = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>(true);
    auto numerical = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>(7);
    const auto boolean_view = ygg::make_view(boolean, *formalism);
    const auto numerical_view = ygg::make_view(numerical, *formalism);
    static_assert(std::same_as<decltype(boolean_view.get()), bool>);
    static_assert(std::same_as<decltype(numerical_view.get()), ygg::uint_t>);
    EXPECT_TRUE(boolean_view.get());
    EXPECT_EQ(numerical_view.get(), 7);
    EXPECT_EQ(&boolean_view.get_handle(), &boolean);
    EXPECT_EQ(&numerical_view.get_context(), formalism.get());

    const auto check = [&]<kr::dl::ConceptOrRoleTag Category>()
    {
        static_assert(std::ranges::forward_range<const kr::dl::semantics::BorrowedDenotationView<Category>>);
        static_assert(std::ranges::forward_range<const kr::dl::semantics::DenotationView<Category>>);
        auto builder = ygg::Builder<kr::dl::semantics::Denotation<Category>>(num_objects);
        const auto borrowed = ygg::make_view(builder, *formalism);
        static_assert(std::same_as<decltype(borrowed.get_handle()), const ygg::Builder<kr::dl::semantics::Denotation<Category>>&>);
        EXPECT_EQ(borrowed.begin(), borrowed.end());
        EXPECT_FALSE(borrowed.any());
        EXPECT_EQ(borrowed.count(), 0);
        EXPECT_EQ(borrowed.get_num_objects(), num_objects);
        EXPECT_EQ(&borrowed.get_formalism_repository(), formalism.get());

        const auto expected = [&]
        {
            if constexpr (std::same_as<Category, kr::dl::ConceptTag>)
            {
                static_assert(std::same_as<decltype(borrowed.get()), ygg::BitsetSpan<const ygg::uint_t>>);
                builder.get().set(0);
                builder.get().set(digits);
                builder.get().set(num_objects - 1);
                return std::array { object(0), object(digits), object(num_objects - 1) };
            }
            else
            {
                static_assert(std::same_as<decltype(borrowed.get(ObjectIndex(0))), ygg::BitsetSpan<const ygg::uint_t>>);
                static_assert(std::same_as<decltype(borrowed.storage_bits()), ygg::BitsetSpan<const ygg::uint_t>>);
                builder.get(0).set(num_objects - 1);
                builder.get(digits).set(1);
                builder.get(num_objects - 1).set(digits);
                return std::array { std::pair(object(0), object(num_objects - 1)),
                                    std::pair(object(digits), object(1)),
                                    std::pair(object(num_objects - 1), object(digits)) };
            }
        }();
        const auto indexed = kr::dl::semantics::insert(repository, builder, scratch).first;
        EXPECT_EQ(indexed.get_num_objects(), num_objects);
        EXPECT_EQ(&indexed.get_formalism_repository(), formalism.get());
        EXPECT_TRUE(borrowed.any());
        EXPECT_EQ(borrowed.count(), expected.size());
        EXPECT_EQ(indexed.count(), expected.size());
        EXPECT_TRUE(std::ranges::equal(borrowed, expected));
        EXPECT_TRUE(std::ranges::equal(indexed, expected));
        EXPECT_TRUE(std::ranges::equal(borrowed.views(), expected));
        EXPECT_TRUE(std::ranges::equal(indexed.views(), expected));
        EXPECT_EQ(borrowed.views().begin(), borrowed.begin());
        EXPECT_EQ(borrowed.views().end(), borrowed.end());
        EXPECT_EQ(indexed.views().begin(), indexed.begin());
        EXPECT_EQ(indexed.views().end(), indexed.end());
        auto borrowed_iterator = ygg::make_view(builder, *formalism).begin();
        auto indexed_iterator = kr::dl::semantics::DenotationView<Category>(indexed).begin();
        for (const auto& value : expected)
        {
            ASSERT_NE(borrowed_iterator, borrowed.end());
            ASSERT_NE(indexed_iterator, indexed.end());
            EXPECT_EQ(*borrowed_iterator++, value);
            EXPECT_EQ(*indexed_iterator++, value);
        }
        EXPECT_EQ(borrowed_iterator, borrowed.end());
        EXPECT_EQ(indexed_iterator, indexed.end());
        if constexpr (std::same_as<Category, kr::dl::RoleTag>)
        {
            const auto borrowed_row = borrowed.views(object(0));
            const auto indexed_row = indexed.views(object(0));
            static_assert(std::ranges::forward_range<decltype(borrowed_row)>);
            static_assert(std::ranges::forward_range<decltype(indexed_row)>);
            static_assert(std::same_as<decltype(borrowed.begin(object(0))), decltype(indexed.begin(object(0)))>);
            static_assert(std::same_as<std::iter_value_t<decltype(borrowed.begin(object(0)))>, tyr::formalism::planning::ObjectView>);
            EXPECT_EQ(borrowed.begin(object(0)), borrowed_row.begin());
            EXPECT_EQ(borrowed.end(object(0)), borrowed_row.end());
            EXPECT_EQ(indexed.begin(object(0)), indexed_row.begin());
            EXPECT_EQ(indexed.end(object(0)), indexed_row.end());
            const auto targets = std::array { object(num_objects - 1) };
            EXPECT_TRUE(std::ranges::equal(borrowed_row, targets));
            EXPECT_TRUE(std::ranges::equal(indexed_row, targets));
            EXPECT_TRUE(borrowed.views(object(1)).empty());
            EXPECT_TRUE(indexed.views(object(1)).empty());
            // Row iterators borrow storage directly, not the temporary view or range.
            auto borrowed_target = ygg::make_view(builder, *formalism).views(object(0)).begin();
            auto indexed_target = kr::dl::semantics::DenotationView<Category>(indexed).views(object(0)).begin();
            EXPECT_EQ(*borrowed_target++, targets.front());
            EXPECT_EQ(*indexed_target++, targets.front());
            EXPECT_EQ(borrowed_target, borrowed_row.end());
            EXPECT_EQ(indexed_target, indexed_row.end());
        }
    };
    check.template operator()<kr::dl::ConceptTag>();
    check.template operator()<kr::dl::RoleTag>();
}

TEST(RunirEvaluationStorage, RegisterInterningUsesRepositoryOwnershipAndPreservesBorrowedValues)
{
    const auto search = query_search();
    const auto formalism = search->task->get_repository();
    const auto objects = search->task->get_domain().get_domain().get_constants();
    auto repository = kr::dl::semantics::DenotationRepositoryFactory().create(formalism);
    auto other_repository = kr::dl::semantics::DenotationRepositoryFactory().create(formalism);
    auto builder = kr::dl::semantics::Builder();
    auto values = ygg::Data<kr::dl::semantics::RegisterValues>();
    values.index = ygg::Index<kr::dl::semantics::RegisterValues>(42);
    values.concept_values.resize(2);
    values.concept_values[0] = objects[0].get_index();
    values.role_values.emplace_back(::cista::pair { objects[1].get_index(), objects[2].get_index() });
    const auto expected = values;
    const auto borrowed = ygg::make_view(values, *formalism);
    const auto [view, inserted] = kr::dl::semantics::copy(borrowed, { repository, builder });
    EXPECT_TRUE(inserted);
    EXPECT_EQ(&view.get_context(), &repository);
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<kr::dl::semantics::RegisterValues>> {}(view.get_data(), expected));
    const auto [duplicate, duplicate_inserted] = kr::dl::semantics::copy(borrowed, { repository, builder });
    EXPECT_FALSE(duplicate_inserted);
    EXPECT_EQ(duplicate, view);
    const auto [same_repository, same_inserted] = kr::dl::semantics::copy(view, { repository, builder });
    EXPECT_FALSE(same_inserted);
    EXPECT_EQ(&same_repository.get_data(), &view.get_data());
    EXPECT_EQ(repository.size<kr::dl::semantics::RegisterValues>(), 1);
    EXPECT_EQ(values.index, expected.index);
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<kr::dl::semantics::RegisterValues>> {}(values, expected));

    auto other_values = expected;
    other_values.concept_values[0] = objects[1].get_index();
    const auto occupied = kr::dl::semantics::insert(other_repository, other_values).first;
    ASSERT_EQ(other_repository.get_index(), repository.get_index());
    ASSERT_EQ(occupied.get_index(), view.get_index());
    const auto [transferred, transferred_inserted] = kr::dl::semantics::copy(view, { other_repository, builder });
    EXPECT_TRUE(transferred_inserted);
    EXPECT_EQ(&transferred.get_context(), &other_repository);
    EXPECT_NE(transferred.get_index(), occupied.get_index());
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<kr::dl::semantics::RegisterValues>> {}(transferred.get_data(), expected));
    const auto [same_target, same_target_inserted] = kr::dl::semantics::copy(view, { other_repository, builder });
    EXPECT_EQ(same_target, transferred);
    EXPECT_FALSE(same_target_inserted);
    EXPECT_EQ(other_repository.size<kr::dl::semantics::RegisterValues>(), 2);

    const auto foreign_search = query_search();
    auto foreign_repository = kr::dl::semantics::DenotationRepositoryFactory().create(foreign_search->task->get_repository());
    EXPECT_THROW((void) kr::dl::semantics::copy(borrowed, { foreign_repository, builder }), std::invalid_argument);
    EXPECT_EQ(foreign_repository.size<kr::dl::semantics::RegisterValues>(), 0);
    auto foreign_values = expected;
    const auto foreign_view = kr::dl::semantics::insert(foreign_repository, foreign_values).first;
    EXPECT_THROW((void) kr::dl::semantics::copy(foreign_view, { repository, builder }), std::invalid_argument);
    EXPECT_EQ(repository.size<kr::dl::semantics::RegisterValues>(), 1);
}

TEST(RunirEvaluationStorage, RegisterExtractionClearsIdentityAndRetainsBuffers)
{
    const auto search = query_search();
    const auto formalism = search->task->get_repository();
    const auto objects = search->task->get_domain().get_domain().get_constants();
    auto repository = kr::dl::semantics::DenotationRepositoryFactory().create(formalism);
    auto values = ygg::Data<kr::dl::semantics::RegisterValues>();
    values.concept_values.resize(2);
    values.concept_values[0] = objects[0].get_index();
    values.role_values.emplace_back(::cista::pair { objects[1].get_index(), objects[2].get_index() });
    const auto view = kr::dl::semantics::insert(repository, values).first;
    auto extracted = ygg::Data<kr::dl::semantics::RegisterValues>();
    extracted.index = view.get_index();
    kr::dl::semantics::assign(extracted, view);
    EXPECT_EQ(extracted.index, ygg::Index<kr::dl::semantics::RegisterValues>());
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<kr::dl::semantics::RegisterValues>> {}(extracted, values));
    const auto concept_buffer = extracted.concept_values.data();
    const auto concept_capacity = extracted.concept_values.allocated_size_;
    const auto role_buffer = extracted.role_values.data();
    const auto role_capacity = extracted.role_values.allocated_size_;

    values.concept_values.resize(1);
    values.concept_values[0] = objects[2].get_index();
    values.role_values[0].reset();
    extracted.index = view.get_index();
    kr::dl::semantics::assign(extracted, ygg::make_view(values, *formalism));
    EXPECT_EQ(extracted.index, ygg::Index<kr::dl::semantics::RegisterValues>());
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<kr::dl::semantics::RegisterValues>> {}(extracted, values));
    EXPECT_EQ(extracted.concept_values.data(), concept_buffer);
    EXPECT_EQ(extracted.concept_values.allocated_size_, concept_capacity);
    EXPECT_EQ(extracted.role_values.data(), role_buffer);
    EXPECT_EQ(extracted.role_values.allocated_size_, role_capacity);
    EXPECT_EQ(view.get_data().concept_values.size(), 2);
    EXPECT_EQ(view.get_data().concept_values[0].value(), objects[0].get_index());
    EXPECT_TRUE(view.get_data().role_values[0].has_value());

    extracted.index = view.get_index();
    kr::dl::semantics::assign(extracted, ygg::make_view(extracted, *formalism));
    EXPECT_EQ(extracted.index, ygg::Index<kr::dl::semantics::RegisterValues>());
    EXPECT_EQ(extracted.concept_values.data(), concept_buffer);
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<kr::dl::semantics::RegisterValues>> {}(extracted, values));
}

TEST(RunirEvaluationStorage, DenotationConversionsResolvePayloadsAndReuseDestinationBuffers)
{
    const auto search = query_search();
    auto source_repository = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto target_repository = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto scratch = kr::dl::semantics::Builder();
    const auto check = [&]<kr::dl::CategoryTag Category>()
    {
        using D = kr::dl::semantics::Denotation<Category>;
        auto payload = ygg::Builder<D>();
        if constexpr (kr::dl::ConceptOrRoleTag<Category>)
        {
            payload.initialize(3);
            if constexpr (std::same_as<Category, kr::dl::ConceptTag>)
                payload.get().set(1);
            else
                payload.get(1).set(2);
        }
        else
            payload.value = 1;
        const auto source = kr::dl::semantics::insert(source_repository, payload, scratch).first;
        auto destination = ygg::Builder<D>();
        kr::dl::semantics::assign(destination, source);
        EXPECT_TRUE(ygg::EqualTo<ygg::Builder<D>> {}(destination, payload));
        EXPECT_EQ(destination.index, ygg::Index<D>());
        if constexpr (kr::dl::ConceptOrRoleTag<Category>)
        {
            const auto* buffer = destination.blocks.data();
            kr::dl::semantics::assign(destination, source);
            EXPECT_EQ(destination.blocks.data(), buffer);
        }

        payload.initialize(0);
        const auto unrelated = kr::dl::semantics::insert(target_repository, payload, scratch).first;
        const auto [copied, created] = kr::dl::semantics::copy(source, { target_repository, scratch });
        EXPECT_TRUE(created);
        EXPECT_NE(copied.get_index(), unrelated.get_index());
        EXPECT_EQ(&copied.get_context(), &target_repository);
        EXPECT_FALSE(kr::dl::semantics::copy(source, { target_repository, scratch }).second);
        EXPECT_EQ(kr::dl::semantics::copy(copied, { target_repository, scratch }).first, copied);

        source_repository.clear();
        kr::dl::semantics::assign(payload, copied);
        EXPECT_TRUE(ygg::EqualTo<ygg::Builder<D>> {}(destination, payload));
    };
    check.template operator()<kr::dl::BooleanTag>();
    check.template operator()<kr::dl::NumericalTag>();
    check.template operator()<kr::dl::ConceptTag>();
    check.template operator()<kr::dl::RoleTag>();
}

TEST(RunirEvaluationStorage, TypedRegisterAssignmentPreservesBuffersAndClearsIdentity)
{
    const auto search = query_search();
    const auto objects = search->task->get_domain().get_domain().get_constants();
    auto values = ygg::Data<kr::dl::semantics::RegisterValues>();
    values.concept_values.resize(2);
    values.role_values.resize(2);
    values.concept_values[0] = objects[0].get_index();
    const auto concept_buffer = values.concept_values.data();
    const auto role_buffer = values.role_values.data();

    values.index = ygg::Index<kr::dl::semantics::RegisterValues>(42);
    kr::dl::semantics::assign_register(values, kr::dl::RegisterIdentifier<kr::dl::ConceptTag>(1), objects[1]);
    EXPECT_EQ(values.index, ygg::Index<kr::dl::semantics::RegisterValues>());
    EXPECT_EQ(values.concept_values[0].value(), objects[0].get_index());
    EXPECT_EQ(values.concept_values[1].value(), objects[1].get_index());

    values.index = ygg::Index<kr::dl::semantics::RegisterValues>(42);
    kr::dl::semantics::assign_register(values, kr::dl::RegisterIdentifier<kr::dl::RoleTag>(1), std::pair(objects[1], objects[2]));
    EXPECT_EQ(values.index, ygg::Index<kr::dl::semantics::RegisterValues>());
    EXPECT_FALSE(values.role_values[0].has_value());
    EXPECT_EQ(values.role_values[1].value().first, objects[1].get_index());
    EXPECT_EQ(values.role_values[1].value().second, objects[2].get_index());
    EXPECT_EQ(values.concept_values.data(), concept_buffer);
    EXPECT_EQ(values.role_values.data(), role_buffer);

    values.index = ygg::Index<kr::dl::semantics::RegisterValues>(42);
    const auto expected = values;
    EXPECT_THROW(kr::dl::semantics::assign_register(values, kr::dl::RegisterIdentifier<kr::dl::ConceptTag>(2), objects[2]), std::out_of_range);
    EXPECT_THROW(kr::dl::semantics::assign_register(values, kr::dl::RegisterIdentifier<kr::dl::RoleTag>(2), std::pair(objects[2], objects[0])),
                 std::out_of_range);
    EXPECT_EQ(values.index, expected.index);
    EXPECT_TRUE(ygg::EqualTo<ygg::Data<kr::dl::semantics::RegisterValues>> {}(values, expected));
}

TEST(RunirEvaluationStorage, DurableRootsDoNotReuseTransientRootEntries)
{
    const auto search = query_search();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto repository = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto persistent = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage<Ext>(persistent);
    auto root_caches = kr::dl::semantics::DenotationCaches<Ext>();
    auto arguments_data = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers_data = ygg::Data<kr::dl::semantics::RegisterValues>();
    registers_data.concept_values.resize(1);
    registers_data.concept_values[0] = search->task->get_domain().get_domain().get_constants()[0].get_index();
    const auto arguments = kr::dl::semantics::insert(persistent, arguments_data).first;
    const auto registers = ygg::make_view(registers_data, *search->task->get_repository());
    using Context =
        kr::dl::semantics::StateEvaluationContext<Ext, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, kr::dl::semantics::BorrowedRegisterValuesView>;
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
            EXPECT_EQ(copy.registers().at(kr::dl::RegisterIdentifier<kr::dl::ConceptTag>(0)).value().get_index(), registers_data.concept_values[0].value());
        }
    }
    EXPECT_EQ(&transient.get_denotation_repository(), &storage.get_denotation_repository(false));
    EXPECT_EQ(&transient.get_caches(), &storage.get_caches());
    EXPECT_EQ(&durable.get_denotation_repository(), &persistent);
    EXPECT_EQ(&durable.get_caches(), &root_caches);
    EXPECT_EQ(&transient.get_workspace(), &builder.get_workspace());
    EXPECT_EQ(&durable.get_workspace(), &builder.get_workspace());

    const auto temporary = kr::dl::semantics::evaluate<tyr::GroundTag>(expression, transient);
    EXPECT_EQ(&temporary.get_context(), &storage.get_denotation_repository(false));
    EXPECT_EQ(persistent.size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
    const auto retained = kr::dl::semantics::evaluate<tyr::GroundTag>(expression, durable);
    EXPECT_EQ(&retained.get_context(), &persistent);
    EXPECT_NE(retained, temporary);
    EXPECT_EQ(retained.count(), temporary.count());
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(expression, durable), retained);
    EXPECT_TRUE(root_caches.get<kr::dl::RoleTag>(false).contains(expression));

    root_caches.reset_dynamic();
    storage.reset_dynamic();
    EXPECT_TRUE(root_caches.get<kr::dl::RoleTag>(false).empty());
    EXPECT_EQ(retained.count(), 4);
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(expression, durable), retained);
    EXPECT_EQ(persistent.size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 1);
    EXPECT_EQ(storage.get_denotation_repository(false).size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
}

TEST(RunirEvaluationStorage, DynamicResetPreservesStaticRowsAndJoinIndexes)
{
    const auto search = query_search();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto constructors = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto prototype = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage<Ext>(prototype);
    auto arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    auto context = kr::dl::semantics::StateEvaluationContext<Ext, tyr::GroundTag>(initial.get_state(),
                                                                                  builder,
                                                                                  storage,
                                                                                  kr::dl::semantics::insert(prototype, arguments).first,
                                                                                  kr::dl::semantics::insert(prototype, registers).first);
    const auto query = [&](const std::string& expression)
    {
        const auto count = kr::ps::ext::dl::parse_numerical("(n_count " + expression + ")", domain, *constructors);
        return count.get_variant()
            .template get<ygg::Index<kr::dl::Numerical<Ext, kr::dl::CountTag>>>()
            .get_arg()
            .template get<ygg::Index<kr::dl::Query<Ext>>>();
    };
    const auto fixed_query = query(R"((q_atomic_state "fixed" (x y z)))");
    const auto renamed_query = query(R"((q_rename (a b c) (q_atomic_state "fixed" (x y z))))");
    const auto dynamic_query = query(R"((q_atomic_state "triple" (x y z)))");
    const auto fixed = kr::dl::semantics::evaluate<tyr::GroundTag>(fixed_query, context);
    const auto alias = kr::dl::semantics::evaluate<tyr::GroundTag>(renamed_query, context);
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(dynamic_query, context).size(), 4);
    const auto& key = fixed.columns()[0];
    const auto keys = std::array { ygg::database::ColumnSlice { key.type, key.offset, key.size } };
    storage.get_caches().get_static_join_indexes().get_or_create(fixed, keys);
    EXPECT_EQ(alias.get_storage_address(), fixed.get_storage_address());
    EXPECT_NE(alias, fixed);

    storage.reset_dynamic();
    EXPECT_TRUE(storage.get_denotation_repository(false).get_relation_repository().empty());
    EXPECT_EQ(fixed.size(), 2);
    EXPECT_EQ(storage.get_caches().get_static_join_indexes().size(), 1);
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(fixed_query, context), fixed);
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(renamed_query, context), alias);
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(dynamic_query, context).size(), 4);

    storage.reset_all();
    EXPECT_TRUE(storage.get_denotation_repository(false).get_relation_repository().empty());
    EXPECT_TRUE(storage.get_denotation_repository(true).get_relation_repository().empty());
    EXPECT_EQ(storage.get_caches().get_static_join_indexes().size(), 0);
    const auto rebuilt = kr::dl::semantics::evaluate<tyr::GroundTag>(fixed_query, context);
    EXPECT_EQ(rebuilt.size(), 2);
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(renamed_query, context).get_storage_address(), rebuilt.get_storage_address());
}

TEST(RunirEvaluationStorage, PreparedContextsRejectForeignTaskRepositories)
{
    const auto search = query_search();
    const auto foreign_search = query_search();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto builder = kr::dl::semantics::Builder();
    auto prototype = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto foreign_prototype = kr::dl::semantics::DenotationRepositoryFactory().create(foreign_search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage<kr::BaseFamilyTag>(prototype);
    auto foreign_storage = kr::dl::semantics::EvaluationStorage<kr::BaseFamilyTag>(foreign_prototype);
    using Context = kr::dl::semantics::StateEvaluationContext<kr::BaseFamilyTag, tyr::GroundTag>;
    auto caches = kr::dl::semantics::DenotationCaches<kr::BaseFamilyTag>();
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
    auto builder = kr::dl::semantics::Builder();
    auto repository = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto input_repository = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto foreign_repository = kr::dl::semantics::DenotationRepositoryFactory().create(foreign_search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage<Ext>(repository);
    auto caches = kr::dl::semantics::DenotationCaches<Ext>();
    auto arguments_data = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers_data = ygg::Data<kr::dl::semantics::RegisterValues>();
    const auto arguments = kr::dl::semantics::insert(input_repository, arguments_data).first;
    const auto foreign_arguments = kr::dl::semantics::insert(foreign_repository, arguments_data).first;
    const auto check = [&](auto registers, auto foreign_registers)
    {
        using Context = kr::dl::semantics::StateEvaluationContext<Ext, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, decltype(registers)>;
        EXPECT_NO_THROW(Context(initial.get_state(), builder, storage, arguments, registers));
        EXPECT_NO_THROW(Context(initial.get_state(), builder, caches, repository, storage, arguments, registers));
        EXPECT_THROW(Context(initial.get_state(), builder, storage, foreign_arguments, registers), std::invalid_argument);
        EXPECT_THROW(Context(initial.get_state(), builder, caches, repository, storage, foreign_arguments, registers), std::invalid_argument);
        EXPECT_THROW(Context(initial.get_state(), builder, storage, arguments, foreign_registers), std::invalid_argument);
        EXPECT_THROW(Context(initial.get_state(), builder, caches, repository, storage, arguments, foreign_registers), std::invalid_argument);
    };
    check(ygg::make_view(registers_data, *search->task->get_repository()), ygg::make_view(registers_data, *foreign_search->task->get_repository()));
    check(kr::dl::semantics::insert(input_repository, registers_data).first, kr::dl::semantics::insert(foreign_repository, registers_data).first);
}

TEST(RunirEvaluationStorage, QueryResultsInternByOrderedNumericSchemaAndRows)
{
    const auto search = query_search();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto factory = kr::dl::ConstructorRepositoryFactoryFor<Ext>();
    auto first_repository = factory.create(search->task->get_repository());
    auto second_repository = factory.create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto persistent = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage<Ext>(persistent);
    auto caches = kr::dl::semantics::DenotationCaches<Ext>();
    auto arguments_data = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers_data = ygg::Data<kr::dl::semantics::RegisterValues>();
    auto context = kr::dl::semantics::StateEvaluationContext<Ext, tyr::GroundTag>(initial.get_state(),
                                                                                  builder,
                                                                                  caches,
                                                                                  persistent,
                                                                                  storage,
                                                                                  kr::dl::semantics::insert(persistent, arguments_data).first,
                                                                                  kr::dl::semantics::insert(persistent, registers_data).first);
    const auto query = [&](const std::string& expression, auto& repository)
    {
        const auto count = kr::ps::ext::dl::parse_numerical("(n_count " + expression + ")", search->task->get_domain().get_domain(), repository);
        return count.get_variant()
            .template get<ygg::Index<kr::dl::Numerical<Ext, kr::dl::CountTag>>>()
            .get_arg()
            .template get<ygg::Index<kr::dl::Query<Ext>>>();
    };
    const auto left = query("(q_role (left_source left_target) (r_universal))", *first_repository);
    const auto right = query("(q_role (right_source right_target) (r_universal))", *second_repository);
    ASSERT_EQ(left.get_columns()[0].get_index(), right.get_columns()[0].get_index());
    ASSERT_NE(left.get_columns()[0].get_name(), right.get_columns()[0].get_name());
    using ColumnIndex = ygg::Index<ygg::database::Column>;
    const auto columns = std::array { ColumnIndex(0), ColumnIndex(1) };
    const auto reversed_columns = std::array { ColumnIndex(1), ColumnIndex(0) };
    auto& results = persistent.get_relation_repository();
    const auto left_result = kr::dl::semantics::evaluate<tyr::GroundTag>(left, context);
    const auto right_result = kr::dl::semantics::evaluate<tyr::GroundTag>(right, context);
    ASSERT_EQ(left_result.size(), 9);
    EXPECT_EQ(ygg::database::column_labels(left_result.columns().span()), std::vector<ColumnIndex>(columns.begin(), columns.end()));
    EXPECT_EQ(&left_result.get_context(), &persistent.get_relation_repository());
    EXPECT_EQ(left_result, right_result);
    EXPECT_EQ(left_result.get_storage_address(), right_result.get_storage_address());
    EXPECT_EQ(results.size(), 1);

    const auto left_alias = query("(q_rename (left_target left_source) (q_role (left_source left_target) (r_universal)))", *first_repository);
    const auto right_alias = query("(q_rename (right_target right_source) (q_role (right_source right_target) (r_universal)))", *second_repository);
    ASSERT_EQ(left_alias.get_columns()[0].get_index(), right_alias.get_columns()[0].get_index());
    const auto renamed_left = kr::dl::semantics::evaluate<tyr::GroundTag>(left_alias, context);
    const auto renamed_right = kr::dl::semantics::evaluate<tyr::GroundTag>(right_alias, context);
    EXPECT_EQ(ygg::database::column_labels(renamed_left.columns().span()), std::vector<ColumnIndex>(reversed_columns.begin(), reversed_columns.end()));
    EXPECT_EQ(renamed_left, renamed_right);
    EXPECT_NE(renamed_left, left_result);
    EXPECT_EQ(renamed_left.get_storage_address(), left_result.get_storage_address());
    EXPECT_EQ(renamed_right.get_storage_address(), right_result.get_storage_address());
    EXPECT_EQ(results.size(), 2);

    {
        auto direct = builder.get_builder<ygg::database::Relation<QueryValues>>(columns);
        for (size_t i = 0; i < left_result.size(); ++i)
            direct->insert(left_result.row(i));
        const auto [interned, inserted] = kr::dl::semantics::insert(persistent, *direct);
        EXPECT_FALSE(inserted);
        EXPECT_EQ(interned, left_result);
    }

    caches.reset_all();
    storage.reset_all();
    first_repository->clear();
    first_repository.reset();
    second_repository.reset();
    EXPECT_EQ(left_result.size(), 9);
    EXPECT_TRUE(left_result.contains(std::tuple { ObjectIndex(0), ObjectIndex(2) }));
    EXPECT_EQ(ygg::database::column_labels(renamed_left.columns().span()), std::vector<ColumnIndex>(reversed_columns.begin(), reversed_columns.end()));
    auto fresh_repository = factory.create(search->task->get_repository());
    const auto fresh = query("(q_role (fresh_source fresh_target) (r_universal))", *fresh_repository);
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(fresh, context), left_result);
    const auto fresh_alias = query("(q_rename (fresh_target fresh_source) (q_role (fresh_source fresh_target) (r_universal)))", *fresh_repository);
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(fresh_alias, context), renamed_left);
    EXPECT_EQ(results.size(), 2);
}

}  // namespace
}  // namespace runir::tests
