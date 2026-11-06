#include <array>
#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/interning.hpp>
#include <string>
#include <tyr/formalism/object_data.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/elements/equal_to.hpp>
#include <urd/elements/hash.hpp>
#include <utility>

namespace runir::tests
{

namespace
{

template<typename T, typename Repository>
concept IndexedDataView =
    std::constructible_from<urd::index<T>, ygg::uint_t> && urd::Identifiable<urd::index<T>> && urd::Identifiable<urd::data<T>>
    && urd::Identifiable<urd::view<T, Repository>> && std::totally_ordered<urd::index<T>> && std::totally_ordered<urd::data<T>>
    && std::totally_ordered<urd::view<T, Repository>> && requires(urd::data<T>& data, const urd::view<T, Repository>& view) {
           data.index;
           view.index();
       };

template<typename Repository, typename... Ts>
consteval bool indexed_data_views(boost::mp11::mp_list<Ts...>)
{ return (IndexedDataView<Ts, Repository> && ...); }

template<typename Category>
using Denotation = kr::dl::semantics::Denotation<Category>;

using DenotationTypes = boost::mp11::mp_transform<Denotation, kr::dl::CategoryTags>;
static_assert(indexed_data_views<kr::dl::semantics::DenotationRepository>(DenotationTypes {}));

using ScalarDenotations = boost::mp11::mp_list<Denotation<kr::dl::BooleanTag>, Denotation<kr::dl::NumericalTag>>;
template<typename T>
consteval bool scalar_denotation()
{
    return requires(urd::data<T>& data, const urd::builder<T>& builder, const urd::view<T, kr::dl::semantics::DenotationRepository>& view) {
        data.value;
        builder.get();
        view.value();
    } && std::same_as<decltype(std::declval<urd::data<T>&>().get_data()), decltype((std::declval<urd::data<T>&>().value))>
           && std::same_as<decltype(std::declval<const urd::data<T>&>().get_data()), decltype((std::declval<const urd::data<T>&>().value))>;
}

template<typename... Ts>
consteval bool scalar_denotations(boost::mp11::mp_list<Ts...>)
{ return (scalar_denotation<Ts>() && ...); }

template<typename... Categories>
consteval bool view_aliases(boost::mp11::mp_list<Categories...>)
{
    return (std::same_as<kr::dl::semantics::DenotationView<Categories>, urd::view<Denotation<Categories>, kr::dl::semantics::DenotationRepository>>
            && ...);
}

static_assert(scalar_denotations(ScalarDenotations {}));
using Concept = Denotation<kr::dl::ConceptTag>;
using ConceptView = kr::dl::semantics::DenotationView<kr::dl::ConceptTag>;
static_assert(requires(urd::data<Concept>& data, const urd::builder<Concept>& builder, const ConceptView& view) {
    data.num_objects;
    data.vec_index;
    builder.get();
    view.value();
    view.begin();
    view.end();
});
using Role = Denotation<kr::dl::RoleTag>;
using RoleView = kr::dl::semantics::DenotationView<kr::dl::RoleTag>;
static_assert(requires(urd::data<Role>& data, const urd::builder<Role>& builder, const RoleView& view, urd::index<tyr::formalism::Object> object) {
    data.num_objects;
    data.vec_index;
    builder.get(object);
    builder.get(ygg::uint_t {});
    builder.get_num_objects();
    builder.storage_bits();
    builder.any();
    builder.count();
    view.value(object);
    view.value(ygg::uint_t {});
    view.get_num_objects();
    view.storage_bits();
    view.any();
    view.count();
    view.begin();
    view.end();
});
static_assert(view_aliases(kr::dl::CategoryTags {}));
static_assert(
    std::same_as<kr::dl::semantics::ConceptDenotationView, urd::view<Denotation<kr::dl::ConceptTag>, kr::dl::semantics::DenotationRepository>>);

}  // namespace

TEST(RunirKrDlSemanticsDenotation, BuilderIdentityUsesValuesRatherThanRegistration)
{
    const auto check = []<kr::dl::CategoryTag Category>()
    {
        using Denotation = kr::dl::semantics::Denotation<Category>;
        auto original = urd::builder<Denotation> {};
        if constexpr (kr::dl::ConceptOrRoleTag<Category>)
            original.initialize(3);
        original.index = urd::index<Denotation>(1);
        auto copy = original;
        copy.index = urd::index<Denotation>(2);

        EXPECT_TRUE(urd::EqualTo<> {}(original, copy));
        EXPECT_EQ(urd::Hash<> {}(original), urd::Hash<> {}(copy));

        if constexpr (kr::dl::ConceptOrRoleTag<Category>)
        {
            copy.blocks.front() = 1;
            const auto larger_universe = urd::builder<Denotation>(4);
            EXPECT_FALSE(urd::EqualTo<> {}(original, larger_universe));
        }
        else
            copy.get() = 1;
        EXPECT_FALSE(urd::EqualTo<> {}(original, copy));
    };
    check.template operator()<kr::dl::BooleanTag>();
    check.template operator()<kr::dl::NumericalTag>();
    check.template operator()<kr::dl::ConceptTag>();
    check.template operator()<kr::dl::RoleTag>();
}

TEST(RunirKrDlSemanticsDenotation, ExposesScalarValues)
{
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = kr::dl::semantics::DenotationRepositoryFactory().create(planning_repository);

    auto boolean_data = urd::data<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>(true);
    auto numerical_data = urd::data<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>(9);
    EXPECT_TRUE(repository.insert(boolean_data).first.value());
    EXPECT_EQ(repository.insert(numerical_data).first.value(), 9);
}

TEST(RunirKrDlSemanticsDenotation, CountsRolePairs)
{
    auto builder = urd::builder<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(3);
    EXPECT_FALSE(builder.any());
    EXPECT_EQ(builder.count(), 0);

    builder.get(0).set(1);
    builder.get(2).set(0);
    EXPECT_TRUE(builder.any());
    EXPECT_EQ(builder.count(), 2);

    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = kr::dl::semantics::DenotationRepositoryFactory().create(planning_repository);
    const auto vec_index = repository.insert_vector(builder.blocks);
    auto data = urd::data<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(builder.get_num_objects(), vec_index);
    const auto view = repository.insert(data).first;

    EXPECT_TRUE(view.any());
    EXPECT_EQ(view.count(), 2);
}

TEST(RunirKrDlSemanticsDenotation, ConceptMutationsClearIdentityAndRetainStorage)
{
    constexpr auto digits = static_cast<ygg::uint_t>(urd::builder<Concept>::Bitset::Digits);
    constexpr auto size = digits + 3;
    const auto object = urd::index<tyr::formalism::Object>(digits);
    auto builder = urd::builder<Concept>(size);
    const auto* storage = builder.blocks.data();
    const auto capacity = builder.blocks.capacity();
    EXPECT_FALSE(builder.any());
    EXPECT_EQ(builder.count(), 0);
    for (const auto& [present, changed] : { std::pair { true, true }, { true, false }, { false, true }, { false, false } })
    {
        builder.index = urd::index<Concept>(7);
        EXPECT_EQ(builder.set(object, present), changed);
        EXPECT_EQ(builder.contains(object), present);
        EXPECT_EQ(builder.any(), present);
        EXPECT_EQ(builder.count(), present ? 1 : 0);
        EXPECT_EQ(builder.index, urd::index<Concept>());
        EXPECT_EQ(builder.blocks.data(), storage);
    }
    builder.set(urd::index<tyr::formalism::Object>(0), true);
    builder.set(object, true);
    builder.index = urd::index<Concept>(7);
    builder.flip();
    EXPECT_EQ(builder.index, urd::index<Concept>());
    EXPECT_TRUE(builder.any());
    EXPECT_EQ(builder.count(), size - 2);
    for (ygg::uint_t i = 0; i < size; ++i)
        EXPECT_EQ(builder.contains(urd::index<tyr::formalism::Object>(i)), i != 0 && i != digits);
    EXPECT_TRUE(builder.get().trailing_bits_zero());
    EXPECT_EQ(builder.storage_bits().count(), builder.count());
    for (size_t i = size; i < builder.storage_bits().size(); ++i)
        EXPECT_FALSE(builder.storage_bits().test(i));
    EXPECT_EQ(builder.blocks.data(), storage);
    EXPECT_EQ(builder.blocks.capacity(), capacity);
}

TEST(RunirKrDlSemanticsDenotation, RoleMutationsClearIdentityAndPreserveRowPadding)
{
    constexpr auto digits = static_cast<ygg::uint_t>(urd::builder<Role>::Bitset::Digits);
    constexpr auto size = digits + 3;
    const auto source = urd::index<tyr::formalism::Object>(size - 1);
    const auto target = urd::index<tyr::formalism::Object>(digits);
    auto builder = urd::builder<Role>(size);
    const auto* storage = builder.blocks.data();
    const auto capacity = builder.blocks.capacity();
    for (const auto& [present, changed] : { std::pair { true, true }, { true, false }, { false, true }, { false, false } })
    {
        builder.index = urd::index<Role>(7);
        EXPECT_EQ(builder.set(source, target, present), changed);
        EXPECT_EQ(builder.contains(source, target), present);
        EXPECT_EQ(builder.any(), present);
        EXPECT_EQ(builder.count(), present ? 1 : 0);
        EXPECT_EQ(builder.index, urd::index<Role>());
        EXPECT_EQ(builder.blocks.data(), storage);
    }
    auto row = urd::builder<Concept>(size);
    row.get().set(0);
    row.get().set(digits);
    builder.set(urd::index<tyr::formalism::Object>(0), source, true);
    builder.index = urd::index<Role>(7);
    builder.assign_row(source, std::as_const(row).get());
    EXPECT_EQ(builder.index, urd::index<Role>());
    EXPECT_EQ(builder.count(), 3);
    EXPECT_EQ(builder.get(source), std::as_const(row).get());
    EXPECT_TRUE(builder.contains(urd::index<tyr::formalism::Object>(0), source));
    EXPECT_TRUE(builder.get(source).trailing_bits_zero());
    EXPECT_EQ(builder.blocks.data(), storage);

    builder.index = urd::index<Role>(7);
    builder.flip();
    EXPECT_EQ(builder.index, urd::index<Role>());
    EXPECT_TRUE(builder.any());
    EXPECT_EQ(builder.count(), static_cast<size_t>(size) * size - 3);
    for (ygg::uint_t i = 0; i < size; ++i)
    {
        EXPECT_TRUE(builder.get(i).trailing_bits_zero());
        for (ygg::uint_t j = 0; j < size; ++j)
        {
            const auto before = (i == size - 1 && (j == 0 || j == digits)) || (i == 0 && j == size - 1);
            EXPECT_EQ(builder.contains(urd::index<tyr::formalism::Object>(i), urd::index<tyr::formalism::Object>(j)), !before);
        }
    }
    EXPECT_EQ(builder.blocks.data(), storage);
    EXPECT_EQ(builder.blocks.capacity(), capacity);
}

TEST(RunirKrDlSemanticsDenotation, BorrowedAssignmentClearsIdentityAndRetainsBuffersIncludingSelfAssignment)
{
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    const auto check = [&]<kr::dl::CategoryTag Category>()
    {
        using D = kr::dl::semantics::Denotation<Category>;
        auto source = urd::builder<D>();
        auto destination = urd::builder<D>();
        if constexpr (kr::dl::ConceptOrRoleTag<Category>)
        {
            constexpr auto size = static_cast<ygg::uint_t>(urd::builder<D>::Bitset::Digits + 3);
            source.initialize(size);
            destination.initialize(size + 1);
            if constexpr (std::same_as<Category, kr::dl::ConceptTag>)
                source.get().set(size - 1);
            else
                source.get(size - 1).set(0);
        }
        else
            source.initialize(7);
        source.index = urd::index<D>(8);
        for (const auto self : { false, true })
        {
            SCOPED_TRACE(self);
            destination.index = urd::index<D>(7);
            const auto view = urd::resolve(self ? destination : source, *planning_repository);
            if constexpr (kr::dl::ConceptOrRoleTag<Category>)
            {
                const auto* storage = destination.blocks.data();
                const auto capacity = destination.blocks.capacity();
                EXPECT_EQ(&kr::dl::semantics::assign(destination, view), &destination);
                EXPECT_EQ(destination.blocks.data(), storage);
                EXPECT_EQ(destination.blocks.capacity(), capacity);
            }
            else
                EXPECT_EQ(&kr::dl::semantics::assign(destination, view), &destination);
            EXPECT_EQ(destination.index, urd::index<D>());
            EXPECT_EQ(source.index, urd::index<D>(8));
            EXPECT_TRUE(urd::EqualTo<> {}(destination, source));
        }
    };
    check.template operator()<kr::dl::BooleanTag>();
    check.template operator()<kr::dl::NumericalTag>();
    check.template operator()<kr::dl::ConceptTag>();
    check.template operator()<kr::dl::RoleTag>();
}

TEST(RunirKrDlSemanticsDenotation, IteratorsOutliveViewWrappers)
{
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    for (const auto* name : { "a", "b", "c" })
    {
        auto data = urd::data<tyr::formalism::Object>(std::string(name));
        (void) planning_repository->insert(data);
    }
    auto repository = kr::dl::semantics::DenotationRepositoryFactory().create(planning_repository);

    auto concept_builder = urd::builder<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(3);
    concept_builder.get().set(0);
    concept_builder.get().set(2);
    auto concept_data = urd::data<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(3, repository.insert_vector(concept_builder.blocks));
    const auto concept_view = repository.insert(concept_data).first;
    const auto concept_copy = concept_view;
    EXPECT_TRUE(concept_view.begin() == concept_copy.begin());
    auto concept_iterator = ConceptView(concept_view).begin();  // The temporary wrapper is gone before iteration.
    const auto concept_end = ConceptView(concept_view).end();
    ASSERT_TRUE(concept_iterator != concept_end);
    EXPECT_EQ((*concept_iterator).index(), urd::index<tyr::formalism::Object>(0));
    ASSERT_TRUE(++concept_iterator != concept_end);
    EXPECT_EQ((*concept_iterator).index(), urd::index<tyr::formalism::Object>(2));
    EXPECT_TRUE(++concept_iterator == concept_end);

    auto role_builder = urd::builder<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(3);
    role_builder.get(0).set(1);
    role_builder.get(2).set(0);
    auto role_data = urd::data<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(3, repository.insert_vector(role_builder.blocks));
    const auto role_view = repository.insert(role_data).first;
    const auto role_copy = role_view;
    EXPECT_TRUE(role_view.begin() == role_copy.begin());
    auto role_iterator = RoleView(role_view).begin();
    const auto role_end = RoleView(role_view).end();
    ASSERT_TRUE(role_iterator != role_end);
    EXPECT_EQ((*role_iterator).first.index(), urd::index<tyr::formalism::Object>(0));
    EXPECT_EQ((*role_iterator).second.index(), urd::index<tyr::formalism::Object>(1));
    ASSERT_TRUE(++role_iterator != role_end);
    EXPECT_EQ((*role_iterator).first.index(), urd::index<tyr::formalism::Object>(2));
    EXPECT_EQ((*role_iterator).second.index(), urd::index<tyr::formalism::Object>(0));
    EXPECT_TRUE(++role_iterator == role_end);
}

TEST(RunirKrDlSemanticsDenotation, IteratorsSurviveRepositoryGrowthAndSkipRolePadding)
{
    using ObjectIndex = urd::index<tyr::formalism::Object>;
    const auto check_indices = [](const auto& indices, const auto& expected)
    {
        auto iterator = indices.begin();
        using Iterator = decltype(iterator);
        static_assert(std::forward_iterator<Iterator>);
        EXPECT_EQ(Iterator {}, Iterator {});
        for (const auto& value : expected)
        {
            ASSERT_NE(iterator, indices.end());
            const auto copy = iterator;
            EXPECT_EQ(copy, iterator);
            EXPECT_EQ(*iterator++, value);
            EXPECT_EQ(*copy, value);
            EXPECT_NE(copy, iterator);
        }
        EXPECT_EQ(iterator, indices.end());
    };
    constexpr ygg::uint_t num_objects = 130;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    for (ygg::uint_t i = 0; i < num_objects; ++i)
    {
        auto data = urd::data<tyr::formalism::Object>("object" + std::to_string(i));
        (void) planning_repository->insert(data);
    }
    auto repository = kr::dl::semantics::DenotationRepositoryFactory().create(planning_repository);
    auto concept_builder = urd::builder<Concept>(num_objects);
    concept_builder.get().set(0);
    concept_builder.get().set(64);
    concept_builder.get().set(129);
    auto concept_data = urd::data<Concept>(num_objects, repository.insert_vector(concept_builder.blocks));
    const auto concept_view = repository.insert(concept_data).first;
    auto concept_iterator = ConceptView(concept_view).begin();
    const auto concept_end = ConceptView(concept_view).end();
    // Only copied iterators survive: both temporary denotation views and indices ranges are gone.
    const auto concept_indices = std::ranges::subrange(ConceptView(concept_view).indices().begin(), ConceptView(concept_view).indices().end());
    const auto borrowed_concept_indices = std::ranges::subrange(urd::resolve(concept_builder, *planning_repository).indices().begin(),
                                                                urd::resolve(concept_builder, *planning_repository).indices().end());
    const auto expected_concept_indices = std::array { ObjectIndex(0), ObjectIndex(64), ObjectIndex(129) };
    static_assert(std::ranges::borrowed_range<decltype(concept_view.indices())>);
    static_assert(std::ranges::forward_range<decltype(concept_indices)>);
    static_assert(std::same_as<std::ranges::range_value_t<decltype(concept_indices)>, ObjectIndex>);
    check_indices(borrowed_concept_indices, expected_concept_indices);

    auto role_builder = urd::builder<Role>(num_objects);
    role_builder.get(0).set(64);
    role_builder.get(129).set(1);
    auto role_data = urd::data<Role>(num_objects, repository.insert_vector(role_builder.blocks));
    const auto role_view = repository.insert(role_data).first;
    auto role_iterator = RoleView(role_view).begin();
    const auto role_end = RoleView(role_view).end();
    const auto role_indices = std::ranges::subrange(RoleView(role_view).indices().begin(), RoleView(role_view).indices().end());
    const auto row_indices = std::ranges::subrange(RoleView(role_view).indices(ObjectIndex(129)).begin(), RoleView(role_view).indices(ObjectIndex(129)).end());
    const auto expected_role_indices = std::array { std::pair(ObjectIndex(0), ObjectIndex(64)), std::pair(ObjectIndex(129), ObjectIndex(1)) };
    const auto expected_row_indices = std::array { ObjectIndex(1) };
    static_assert(std::ranges::borrowed_range<decltype(role_view.indices())>);
    static_assert(std::ranges::borrowed_range<decltype(role_view.indices(ObjectIndex(129)))>);
    static_assert(std::ranges::forward_range<decltype(role_indices)>);
    static_assert(std::same_as<std::ranges::range_value_t<decltype(role_indices)>, std::pair<ObjectIndex, ObjectIndex>>);
    static_assert(std::same_as<std::ranges::range_value_t<decltype(row_indices)>, ObjectIndex>);
    const auto borrowed_role = urd::resolve(role_builder, *planning_repository);
    const auto borrowed_role_indices = std::ranges::subrange(urd::resolve(role_builder, *planning_repository).indices().begin(),
                                                             urd::resolve(role_builder, *planning_repository).indices().end());
    const auto borrowed_row_indices = std::ranges::subrange(urd::resolve(role_builder, *planning_repository).indices(ObjectIndex(129)).begin(),
                                                            urd::resolve(role_builder, *planning_repository).indices(ObjectIndex(129)).end());
    static_assert(std::ranges::borrowed_range<decltype(borrowed_role.indices())>);
    check_indices(borrowed_role_indices, expected_role_indices);
    check_indices(borrowed_row_indices, expected_row_indices);
    EXPECT_TRUE(std::ranges::empty(borrowed_role.indices(ObjectIndex(1))));
    EXPECT_TRUE(std::ranges::empty(role_view.indices(ObjectIndex(1))));

    for (ygg::uint_t i = 0; i < 1024; ++i)
    {
        concept_builder.initialize(num_objects);
        concept_builder.blocks.front() = i;
        auto data = urd::data<Concept>(num_objects, repository.insert_vector(concept_builder.blocks));
        (void) repository.insert(data);
    }

    check_indices(concept_indices, expected_concept_indices);
    check_indices(role_indices, expected_role_indices);
    check_indices(row_indices, expected_row_indices);
    EXPECT_TRUE(concept_iterator == concept_view.begin());
    for (const auto expected : { 0U, 64U, 129U })
    {
        ASSERT_TRUE(concept_iterator != concept_end);
        EXPECT_EQ((*concept_iterator).index(), urd::index<tyr::formalism::Object>(expected));
        ++concept_iterator;
    }
    EXPECT_TRUE(concept_iterator == concept_end);
    EXPECT_TRUE(role_iterator == role_view.begin());
    ASSERT_TRUE(role_iterator != role_end);
    EXPECT_EQ((*role_iterator).first.index(), urd::index<tyr::formalism::Object>(0));
    EXPECT_EQ((*role_iterator).second.index(), urd::index<tyr::formalism::Object>(64));
    ASSERT_TRUE(++role_iterator != role_end);
    EXPECT_EQ((*role_iterator).first.index(), urd::index<tyr::formalism::Object>(129));
    EXPECT_EQ((*role_iterator).second.index(), urd::index<tyr::formalism::Object>(1));
    EXPECT_TRUE(++role_iterator == role_end);
}

TEST(RunirKrDlSemanticsDenotation, RoleStorageBitsetsPreserveRowPadding)
{
    using RoleBuilder = urd::builder<Role>;
    constexpr auto digits = static_cast<ygg::uint_t>(RoleBuilder::Bitset::Digits);
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    for (ygg::uint_t i = 0; i < 130; ++i)
    {
        auto data = urd::data<tyr::formalism::Object>("object" + std::to_string(i));
        (void) planning_repository->insert(data);
    }
    auto repository = kr::dl::semantics::DenotationRepositoryFactory().create(planning_repository);
    for (const auto size : { ygg::uint_t { 0 }, ygg::uint_t { 1 }, digits - 1, digits, digits + 1, ygg::uint_t { 130 } })
    {
        SCOPED_TRACE(size);
        auto lhs = RoleBuilder(size);
        auto rhs = RoleBuilder(size);
        auto result = RoleBuilder(size);
        EXPECT_FALSE(result.any());
        EXPECT_EQ(result.count(), 0);
        EXPECT_EQ(result.storage_bits().size(), result.blocks.size() * RoleBuilder::Bitset::Digits);
        const auto* storage = result.blocks.data();
        for (ygg::uint_t source = 0; source < size; ++source)
            for (ygg::uint_t target = 0; target < size; ++target)
            {
                lhs.get(source).set(target, (source + 2 * target) % 7 == 0);
                rhs.get(source).set(target, (2 * source + target) % 11 == 0);
            }

        for (const auto intersection : { false, true })
        {
            result.storage_bits().copy_from(std::as_const(lhs).storage_bits());
            if (intersection)
                result.storage_bits() &= std::as_const(rhs).storage_bits();
            else
                result.storage_bits() |= std::as_const(rhs).storage_bits();
            EXPECT_EQ(result.blocks.data(), storage);
            auto count = size_t { 0 };
            for (ygg::uint_t source = 0; source < size; ++source)
            {
                EXPECT_TRUE(result.get(source).trailing_bits_zero());
                for (ygg::uint_t target = 0; target < size; ++target)
                {
                    const auto left = lhs.get(source).test(target);
                    const auto right = rhs.get(source).test(target);
                    const auto expected = intersection ? left && right : left || right;
                    EXPECT_EQ(result.get(source).test(target), expected);
                    count += expected;
                }
            }
            EXPECT_EQ(result.count(), count);
            EXPECT_EQ(result.any(), count != 0);
            auto data = urd::data<Role>(size, repository.insert_vector(result.blocks));
            const auto view = repository.insert(data).first;
            EXPECT_EQ(view.storage_bits(), std::as_const(result).storage_bits());
            EXPECT_EQ(view.count(), count);
            EXPECT_EQ(view.any(), count != 0);
            EXPECT_EQ(std::ranges::distance(view.indices()), count);
            EXPECT_EQ(std::ranges::distance(urd::resolve(result, *planning_repository).indices()), count);
            auto iterated = size_t { 0 };
            for (const auto [source, target] : view)
            {
                EXPECT_LT(ygg::uint_t(source.index().value), size);
                EXPECT_LT(ygg::uint_t(target.index().value), size);
                EXPECT_TRUE(result.get(source.index()).test(ygg::uint_t(target.index().value)));
                ++iterated;
            }
            EXPECT_EQ(iterated, count);
        }
    }
}
}
