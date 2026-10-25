#include <array>
#include <gtest/gtest.h>
#include <random>
#include <runir/kr/dl/semantics/incremental/detail/set_operations.hpp>
#include <string>
#include <tyr/formalism/object_data.hpp>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::tests
{
namespace
{

template<ygg::uint_t size>
void check_composition_and_closures()
{
    SCOPED_TRACE(size);
    constexpr auto cells = std::size_t { size } * size;
    auto repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    for (ygg::uint_t i = 0; i < size; ++i)
    {
        auto data = ygg::Data<tyr::formalism::Object>("o" + std::to_string(i));
        (void) repository->insert(data);
    }
    const auto object = [&](ygg::uint_t index) { return ygg::make_view(ygg::Index<tyr::formalism::Object>(index), *repository); };
    auto lhs = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(size);
    auto rhs = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(size);
    for (ygg::uint_t i = 0; i < size; ++i)
    {
        lhs.get(i).set((i + 1) % size);
        rhs.get(i).set((i + 2) % size);
    }
    auto previous_lhs = lhs;
    auto previous_rhs = rhs;
    auto composition = kr::dl::semantics::incremental::detail::DenotationState<kr::dl::RoleTag>();
    auto transitive = kr::dl::semantics::incremental::detail::DenotationState<kr::dl::RoleTag>();
    auto reflexive = kr::dl::semantics::incremental::detail::DenotationState<kr::dl::RoleTag>();
    auto workspace = kr::dl::semantics::incremental::detail::SetOperationWorkspace();
    workspace.initialize(size);
    const auto lhs_view = ygg::make_view(lhs, *repository);
    const auto rhs_view = ygg::make_view(rhs, *repository);

    const auto check_result = [&](const kr::dl::semantics::incremental::detail::DenotationState<kr::dl::RoleTag>& output,
                                  const ygg::Builder<kr::dl::semantics::Denotation<kr::dl::RoleTag>>& previous,
                                  const std::array<bool, cells>& expected,
                                  bool initialized)
    {
        auto added = std::array<bool, cells> {};
        auto removed = std::array<bool, cells> {};
        for (const auto& [source, target] : output.get_delta().added)
        {
            const auto position = static_cast<ygg::uint_t>(source.get_index()) * size + static_cast<ygg::uint_t>(target.get_index());
            ASSERT_LT(position, added.size());
            EXPECT_FALSE(added[position]);
            added[position] = true;
        }
        for (const auto& [source, target] : output.get_delta().removed)
        {
            const auto position = static_cast<ygg::uint_t>(source.get_index()) * size + static_cast<ygg::uint_t>(target.get_index());
            ASSERT_LT(position, removed.size());
            EXPECT_FALSE(removed[position]);
            removed[position] = true;
        }
        size_t count = 0;
        for (ygg::uint_t source = 0; source < size; ++source)
        {
            EXPECT_TRUE(output.get_builder().get(source).trailing_bits_zero());
            for (ygg::uint_t target = 0; target < size; ++target)
            {
                const auto position = source * size + target;
                const auto before = previous.get(source)[target];
                EXPECT_EQ(output.get_builder().get(source)[target], expected[position]);
                EXPECT_EQ(added[position], !initialized && !before && expected[position]);
                EXPECT_EQ(removed[position], !initialized && before && !expected[position]);
                count += expected[position];
            }
        }
        EXPECT_EQ(output.size(), count);
    };

    const auto check = [&](bool initialize = false)
    {
        auto lhs_delta = kr::dl::semantics::incremental::DenotationDelta<kr::dl::RoleTag>();
        auto rhs_delta = kr::dl::semantics::incremental::DenotationDelta<kr::dl::RoleTag>();
        for (ygg::uint_t source = 0; source < size; ++source)
            for (ygg::uint_t target = 0; target < size; ++target)
            {
                if (lhs.get(source)[target] != previous_lhs.get(source)[target])
                    (lhs.get(source)[target] ? lhs_delta.added : lhs_delta.removed).emplace_back(object(source), object(target));
                if (rhs.get(source)[target] != previous_rhs.get(source)[target])
                    (rhs.get(source)[target] ? rhs_delta.added : rhs_delta.removed).emplace_back(object(source), object(target));
            }
        const auto old_composition = initialize ? ygg::Builder<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(size) : composition.get_builder();
        const auto old_transitive = initialize ? ygg::Builder<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(size) : transitive.get_builder();
        const auto old_reflexive = initialize ? ygg::Builder<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(size) : reflexive.get_builder();
        if (initialize)
        {
            kr::dl::semantics::incremental::detail::initialize_set(kr::dl::CompositionTag {}, composition, lhs_view, lhs_delta, rhs_view, rhs_delta, workspace);
            kr::dl::semantics::incremental::detail::initialize_set(kr::dl::TransitiveClosureTag {}, transitive, lhs_view, lhs_delta, workspace);
            kr::dl::semantics::incremental::detail::initialize_set(kr::dl::ReflexiveTransitiveClosureTag {}, reflexive, lhs_view, lhs_delta, workspace);
        }
        else
        {
            kr::dl::semantics::incremental::detail::update_set(kr::dl::CompositionTag {}, composition, lhs_view, lhs_delta, rhs_view, rhs_delta, workspace);
            kr::dl::semantics::incremental::detail::update_set(kr::dl::TransitiveClosureTag {}, transitive, lhs_view, lhs_delta, workspace);
            kr::dl::semantics::incremental::detail::update_set(kr::dl::ReflexiveTransitiveClosureTag {}, reflexive, lhs_view, lhs_delta, workspace);
        }

        auto expected_composition = std::array<bool, cells> {};
        auto expected_closure = std::array<bool, cells> {};
        for (ygg::uint_t source = 0; source < size; ++source)
            for (ygg::uint_t target = 0; target < size; ++target)
            {
                expected_closure[source * size + target] = lhs.get(source)[target];
                for (ygg::uint_t middle = 0; middle < size; ++middle)
                    if (lhs.get(source)[middle] && rhs.get(middle)[target])
                        expected_composition[source * size + target] = true;
            }
        // Independent Floyd-Warshall reference, including nonempty paths to self.
        for (ygg::uint_t middle = 0; middle < size; ++middle)
            for (ygg::uint_t source = 0; source < size; ++source)
                for (ygg::uint_t target = 0; target < size; ++target)
                    if (expected_closure[source * size + middle] && expected_closure[middle * size + target])
                        expected_closure[source * size + target] = true;
        check_result(composition, old_composition, expected_composition, initialize);
        check_result(transitive, old_transitive, expected_closure, initialize);
        for (ygg::uint_t object = 0; object < size; ++object)
            expected_closure[object * size + object] = true;
        check_result(reflexive, old_reflexive, expected_closure, initialize);
        previous_lhs = lhs;
        previous_rhs = rhs;
    };
    check(true);
    for (const auto* output : { &composition, &transitive, &reflexive })
    {
        EXPECT_EQ(output->get_delta().added.capacity(), 0);
        EXPECT_EQ(output->get_delta().removed.capacity(), 0);
    }

    if constexpr (size >= 5)
    {
        // Repeated changed middle objects must be discovered once, including simultaneous left changes.
        rhs.get(1).set(0);
        rhs.get(1).set(1);
        rhs.get(1).set(size - 1);
        lhs.get(0).set(1);
        lhs.get(2).set(1);
        check();
        rhs.get(1).reset(0);
        rhs.get(1).reset(1);
        rhs.get(2).set(size - 1);
        lhs.get(0).reset(1);
        check();

        SCOPED_TRACE("directed SCC splits, merges, and a clean suffix");
        lhs.storage_bits().reset();
        lhs.get(0).set(1);
        lhs.get(1).set(2);
        lhs.get(2).set(1);
        lhs.get(2).set(3);
        lhs.get(3).set(size - 1);
        check();

        // The surviving cycle must not preserve reachability from the disconnected source.
        lhs.get(0).reset(1);
        check();
        lhs.get(0).set(1);
        check();

        // Split and merge the cycle while the downstream suffix stays clean.
        lhs.get(2).reset(1);
        check();
        lhs.get(2).set(1);
        check();

        // Merge the suffix into the cycle, then split it back out.
        lhs.get(size - 1).set(1);
        check();
        lhs.get(size - 1).reset(1);
        check();
        check();
    }

    auto random = std::mt19937(1009);
    for (size_t iteration = 0; iteration < 200; ++iteration)
    {
        SCOPED_TRACE(iteration);
        const auto saved_lhs = lhs;
        const auto saved_rhs = rhs;
        for (size_t change = 0; change < 20; ++change)
        {
            const auto source = static_cast<ygg::uint_t>(random() % size);
            const auto target = static_cast<ygg::uint_t>(random() % size);
            (random() % 2 == 0 ? lhs : rhs).get(source).set(target, random() % 5 == 0);
        }
        if (iteration % 23 == 0)
            lhs.storage_bits().reset();
        if (iteration % 29 == 0)
            for (ygg::uint_t object = 0; object < size; ++object)
                lhs.get(object).set((object + 1) % size);
        check();
        check();  // No changes must clear previous output deltas.
        if (iteration % 3 == 0)
        {
            lhs = saved_lhs;
            rhs = saved_rhs;
            check();
        }
    }
}

TEST(RunirIncrementalSetOperations, CompositionAndClosuresMatchFreshResultsAndExactDeltas)
{
    check_composition_and_closures<1>();
    check_composition_and_closures<10>();
    check_composition_and_closures<static_cast<ygg::uint_t>(ygg::BitsetSpan<const ygg::uint_t>::Digits + 3)>();
}

TEST(RunirIncrementalSetOperations, ConceptIntersectionAndUnionAcrossWordBoundaries)
{
    constexpr auto digits = static_cast<ygg::uint_t>(ygg::BitsetSpan<const ygg::uint_t>::Digits), size = digits + 2;
    auto repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    for (ygg::uint_t i = 0; i < size; ++i)
    {
        auto data = ygg::Data<tyr::formalism::Object>("o" + std::to_string(i));
        (void) repository->insert(data);
    }
    const auto object = [&](ygg::uint_t i) { return ygg::make_view(ygg::Index<tyr::formalism::Object>(i), *repository); };
    const auto check = [&](auto tag)
    {
        constexpr auto intersection = std::same_as<decltype(tag), kr::dl::IntersectionTag>;
        SCOPED_TRACE(intersection);
        auto lhs = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(size), rhs = lhs;
        for (const auto i : { digits - 1, digits, digits + 1 })
            lhs.get().set(i);
        for (const auto i : { ygg::uint_t { 0 }, digits, digits + 1 })
            rhs.get().set(i);
        const auto left = ygg::make_view(lhs, *repository), right = ygg::make_view(rhs, *repository);
        auto lhs_delta = kr::dl::semantics::incremental::DenotationDelta<kr::dl::ConceptTag>(), rhs_delta = lhs_delta;
        auto output = kr::dl::semantics::incremental::detail::DenotationState<kr::dl::ConceptTag>();
        auto workspace = kr::dl::semantics::incremental::detail::SetOperationWorkspace();
        workspace.initialize(size);
        kr::dl::semantics::incremental::detail::initialize_set(tag, output, left, lhs_delta, right, rhs_delta, workspace);
        EXPECT_TRUE(output.get_delta().empty());
        EXPECT_EQ(output.size(), intersection ? 2 : 4);
        for (ygg::uint_t i = 0; i < size; ++i)
            EXPECT_EQ(output.contains(object(i)), i == digits || i == digits + 1 || (!intersection && (i == 0 || i == digits - 1)));
        lhs.get().reset(digits + 1);
        lhs.get().set(1);
        rhs.get().reset(digits + 1);
        rhs.get().set(digits - 1);
        lhs_delta.removed = rhs_delta.removed = { object(digits + 1) };
        lhs_delta.added = { object(1) };
        rhs_delta.added = { object(digits - 1) };
        kr::dl::semantics::incremental::detail::update_set(tag, output, left, lhs_delta, right, rhs_delta, workspace);
        EXPECT_EQ(output.get_delta().added, std::vector { object(intersection ? digits - 1 : 1) });
        EXPECT_EQ(output.get_delta().removed, std::vector { object(digits + 1) });
        for (ygg::uint_t i = 0; i < size; ++i)
            EXPECT_EQ(output.contains(object(i)), i == digits || i == digits - 1 || (!intersection && (i == 0 || i == 1)));
        EXPECT_EQ(output.size(), intersection ? 2 : 4);
        EXPECT_TRUE(output.get_builder().get().trailing_bits_zero());
    };
    check(kr::dl::IntersectionTag {});
    check(kr::dl::UnionTag {});
}

TEST(RunirIncrementalSetOperations, RestrictionBatchesConceptChangesWithChangedRoleRows)
{
    constexpr auto size = static_cast<ygg::uint_t>(ygg::BitsetSpan<const ygg::uint_t>::Digits + 3);
    auto repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    for (ygg::uint_t i = 0; i < size; ++i)
    {
        auto data = ygg::Data<tyr::formalism::Object>("o" + std::to_string(i));
        (void) repository->insert(data);
    }
    const auto object = [&](ygg::uint_t i) { return ygg::make_view(ygg::Index<tyr::formalism::Object>(i), *repository); };
    auto role = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(size);
    auto concept_ = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(size);
    role.get(0).set(1);
    role.get(1).set(size - 1);
    role.get(2).set(1);
    role.get(2).set(size - 1);
    role.get(3).set(2);
    concept_.get().set(1);
    auto role_delta = kr::dl::semantics::incremental::DenotationDelta<kr::dl::RoleTag>();
    auto concept_delta = kr::dl::semantics::incremental::DenotationDelta<kr::dl::ConceptTag>();
    auto restricted = kr::dl::semantics::incremental::detail::DenotationState<kr::dl::RoleTag>();
    auto existential = kr::dl::semantics::incremental::detail::DenotationState<kr::dl::ConceptTag>();
    auto workspace = kr::dl::semantics::incremental::detail::SetOperationWorkspace();
    workspace.initialize(size);
    const auto role_view = ygg::make_view(role, *repository);
    const auto concept_view = ygg::make_view(concept_, *repository);
    kr::dl::semantics::incremental::detail::initialize_set(kr::dl::RestrictionTag {},
                                                           restricted,
                                                           role_view,
                                                           role_delta,
                                                           concept_view,
                                                           concept_delta,
                                                           workspace);
    kr::dl::semantics::incremental::detail::initialize_set(kr::dl::ExistentialQuantificationTag {},
                                                           existential,
                                                           role_view,
                                                           role_delta,
                                                           concept_view,
                                                           concept_delta,
                                                           workspace);

    role.get(0).reset(1);
    role.get(0).set(size - 1);
    role_delta.removed.emplace_back(object(0), object(1));
    role_delta.added.emplace_back(object(0), object(size - 1));
    concept_.get().reset(1);
    concept_.get().set(size - 1);
    concept_delta.removed.push_back(object(1));
    concept_delta.added.push_back(object(size - 1));
    kr::dl::semantics::incremental::detail::update_set(kr::dl::RestrictionTag {}, restricted, role_view, role_delta, concept_view, concept_delta, workspace);
    kr::dl::semantics::incremental::detail::update_set(kr::dl::ExistentialQuantificationTag {},
                                                       existential,
                                                       role_view,
                                                       role_delta,
                                                       concept_view,
                                                       concept_delta,
                                                       workspace);
    for (ygg::uint_t source = 0; source < size; ++source)
    {
        EXPECT_EQ(existential.get_builder().get().test(source), source < 3);
        for (ygg::uint_t target = 0; target < size; ++target)
            EXPECT_EQ(restricted.get_builder().get(source).test(target), source < 3 && target == size - 1);
    }
    EXPECT_EQ(restricted.get_delta().added.size(), 3);
    EXPECT_EQ(restricted.get_delta().removed.size(), 2);
    EXPECT_EQ(existential.get_delta().added, std::vector { object(1) });
    EXPECT_TRUE(existential.get_delta().removed.empty());
}

TEST(RunirIncrementalSetOperations, InverseAndComplementPreservePaddingAcrossWordBoundaries)
{
    constexpr auto size = static_cast<ygg::uint_t>(ygg::BitsetSpan<const ygg::uint_t>::Digits + 3);
    auto repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    for (ygg::uint_t i = 0; i < size; ++i)
    {
        auto data = ygg::Data<tyr::formalism::Object>("o" + std::to_string(i));
        (void) repository->insert(data);
    }
    const auto object = [&](ygg::uint_t index) { return ygg::make_view(ygg::Index<tyr::formalism::Object>(index), *repository); };
    auto role = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(size);
    role.get(0).set(size - 1);
    role.get(size - 1).set(1);
    auto inverse = kr::dl::semantics::incremental::detail::DenotationState<kr::dl::RoleTag>();
    auto complement = kr::dl::semantics::incremental::detail::DenotationState<kr::dl::RoleTag>();
    auto delta = kr::dl::semantics::incremental::DenotationDelta<kr::dl::RoleTag>();
    auto workspace = kr::dl::semantics::incremental::detail::SetOperationWorkspace();
    workspace.initialize(size);
    const auto view = ygg::make_view(role, *repository);
    kr::dl::semantics::incremental::detail::initialize_set(kr::dl::InverseTag {}, inverse, view, delta, workspace);
    kr::dl::semantics::incremental::detail::initialize_set(kr::dl::ComplementTag {}, complement, view, delta, workspace);
    for (const auto* output : { &inverse, &complement })
    {
        EXPECT_TRUE(output->get_delta().empty());
        EXPECT_EQ(output->get_delta().added.capacity(), 0);
        EXPECT_EQ(output->get_delta().removed.capacity(), 0);
    }
    role.get(0).reset(size - 1);
    role.get(size - 2).set(size - 1);
    delta.removed.emplace_back(object(0), object(size - 1));
    delta.added.emplace_back(object(size - 2), object(size - 1));
    kr::dl::semantics::incremental::detail::update_set(kr::dl::InverseTag {}, inverse, view, delta, workspace);
    kr::dl::semantics::incremental::detail::update_set(kr::dl::ComplementTag {}, complement, view, delta, workspace);
    EXPECT_EQ(inverse.size(), 2);
    EXPECT_EQ(complement.size(), static_cast<size_t>(size) * size - 2);
    ASSERT_EQ(inverse.get_delta().added.size(), 1);
    ASSERT_EQ(inverse.get_delta().removed.size(), 1);
    ASSERT_EQ(complement.get_delta().added.size(), 1);
    ASSERT_EQ(complement.get_delta().removed.size(), 1);
    EXPECT_EQ(inverse.get_delta().added.front(), std::pair(object(size - 1), object(size - 2)));
    EXPECT_EQ(inverse.get_delta().removed.front(), std::pair(object(size - 1), object(0)));
    EXPECT_EQ(complement.get_delta().added.front(), delta.removed.front());
    EXPECT_EQ(complement.get_delta().removed.front(), delta.added.front());
    const auto check_result = [&]
    {
        for (ygg::uint_t source = 0; source < size; ++source)
        {
            EXPECT_TRUE(inverse.get_builder().get(source).trailing_bits_zero());
            EXPECT_TRUE(complement.get_builder().get(source).trailing_bits_zero());
            for (ygg::uint_t target = 0; target < size; ++target)
            {
                EXPECT_EQ(inverse.get_builder().get(source)[target], role.get(target)[source]);
                EXPECT_EQ(complement.get_builder().get(source)[target], !role.get(source)[target]);
            }
        }
    };
    check_result();
    const auto inverse_capacity = std::pair(inverse.get_delta().added.capacity(), inverse.get_delta().removed.capacity());
    const auto complement_capacity = std::pair(complement.get_delta().added.capacity(), complement.get_delta().removed.capacity());
    role.get(2).set(3);
    kr::dl::semantics::incremental::detail::initialize_set(kr::dl::InverseTag {}, inverse, view, delta, workspace);
    kr::dl::semantics::incremental::detail::initialize_set(kr::dl::ComplementTag {}, complement, view, delta, workspace);
    EXPECT_TRUE(inverse.get_delta().empty());
    EXPECT_TRUE(complement.get_delta().empty());
    EXPECT_EQ(std::pair(inverse.get_delta().added.capacity(), inverse.get_delta().removed.capacity()), inverse_capacity);
    EXPECT_EQ(std::pair(complement.get_delta().added.capacity(), complement.get_delta().removed.capacity()), complement_capacity);
    EXPECT_EQ(inverse.size(), 3);
    EXPECT_EQ(complement.size(), static_cast<size_t>(size) * size - 3);
    check_result();
}

}  // namespace
}  // namespace runir::tests
