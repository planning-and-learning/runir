#include "planning_fixtures.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <filesystem>
#include <gtest/gtest.h>
#include <iterator>
#include <ranges>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/incremental/delta.hpp>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <utility>
#include <vector>

namespace runir::tests
{
namespace
{
namespace dl = kr::dl;
namespace sem = dl::semantics;
namespace fp = tyr::formalism::planning;
using Delta = sem::incremental::Delta<kr::ExtFamilyTag>;
using Object = ygg::Index<tyr::formalism::Object>;
using ConceptRegister = dl::RegisterIdentifier<dl::ConceptTag>;
using RoleRegister = dl::RegisterIdentifier<dl::RoleTag>;

template<typename Changes>
concept HasRegisterChanges = requires(Changes delta) {
    delta.added.concept_registers;
    delta.added.role_registers;
};

static_assert(!HasRegisterChanges<sem::incremental::Delta<kr::BaseFamilyTag>>);
static_assert(!HasRegisterChanges<sem::incremental::Delta<kr::UnsFamilyTag>>);
static_assert(HasRegisterChanges<Delta>);

template<tyr::TaskKind Kind, tyr::formalism::FactKind Fact, typename State>
auto atoms(const State& state)
{
    auto result = std::vector<fp::AtomView<tyr::GroundTag, Fact>> {};
    for (const auto atom : state.template get_atoms_view<Fact>())
        result.push_back(atom);
    std::ranges::sort(result);
    return result;
}

template<typename Values>
auto difference(const Values& lhs, const Values& rhs)
{
    auto result = Values {};
    std::ranges::set_difference(lhs, rhs, std::back_inserter(result));
    return result;
}

auto capacities(const Delta& delta)
{
    return std::array { delta.added.fluent_atoms.capacity(),    delta.removed.fluent_atoms.capacity(),    delta.added.derived_atoms.capacity(),
                        delta.removed.derived_atoms.capacity(), delta.added.concept_registers.capacity(), delta.removed.concept_registers.capacity(),
                        delta.added.role_registers.capacity(),  delta.removed.role_registers.capacity() };
}

template<typename Side>
void expect_same_side(const Side& lhs, const Side& rhs)
{
    EXPECT_TRUE(std::ranges::is_permutation(lhs.fluent_atoms, rhs.fluent_atoms));
    EXPECT_TRUE(std::ranges::is_permutation(lhs.derived_atoms, rhs.derived_atoms));
    EXPECT_EQ(lhs.concept_registers, rhs.concept_registers);
    EXPECT_EQ(lhs.role_registers, rhs.role_registers);
}

template<dl::FamilyTag Family, tyr::TaskKind Kind>
void check_state_delta(tyr::planning::StateView<Kind> source, tyr::planning::StateView<Kind> target)
{
    auto delta = sem::incremental::Delta<Family> {};
    const auto source_builder = source.get_state_builder();
    const auto target_builder = target.get_state_builder();
    const auto borrowed_source = ygg::make_view(source_builder, source.get_task());
    const auto borrowed_target = ygg::make_view(target_builder, target.get_task());
    delta.template assign<Kind>(borrowed_source, borrowed_target);
    EXPECT_FALSE(delta.empty());
    const auto expect_atoms = [&](const auto& before, const auto& after)
    {
        const auto before_fluent = atoms<Kind, tyr::formalism::FluentTag>(before);
        const auto after_fluent = atoms<Kind, tyr::formalism::FluentTag>(after);
        const auto before_derived = atoms<Kind, tyr::formalism::DerivedTag>(before);
        const auto after_derived = atoms<Kind, tyr::formalism::DerivedTag>(after);
        EXPECT_TRUE(std::ranges::is_permutation(delta.added.fluent_atoms, difference(after_fluent, before_fluent)));
        EXPECT_TRUE(std::ranges::is_permutation(delta.removed.fluent_atoms, difference(before_fluent, after_fluent)));
        EXPECT_TRUE(std::ranges::is_permutation(delta.added.derived_atoms, difference(after_derived, before_derived)));
        EXPECT_TRUE(std::ranges::is_permutation(delta.removed.derived_atoms, difference(before_derived, after_derived)));
    };
    expect_atoms(source, target);
    delta.reverse();
    expect_atoms(target, source);
    const auto capacities = [&]
    { return std::array { delta.added.fluent_atoms.capacity(), delta.removed.fluent_atoms.capacity(),
                         delta.added.derived_atoms.capacity(), delta.removed.derived_atoms.capacity() }; };
    const auto reserved = capacities();
    delta.clear();
    EXPECT_TRUE(delta.empty());
    EXPECT_EQ(capacities(), reserved);
    delta.template assign<Kind>(target, source);
    expect_atoms(target, source);
    EXPECT_EQ(capacities(), reserved);
    const auto other_task = source.get_task();
    EXPECT_THROW(delta.template assign<Kind>(source, ygg::make_view(source_builder, other_task)), std::invalid_argument);
    expect_atoms(target, source);
    delta.template assign<Kind>(source, source);
    EXPECT_TRUE(delta.empty());
}

template<tyr::TaskKind Kind>
void check_delta()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/dl/query";
    const auto search = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
        else
            return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
    }();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto successors = search->successor_generator->get_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator);
    ASSERT_FALSE(successors.empty());
    const auto source = initial.get_state();
    const auto target = successors.front().get_state();
    const auto source_fluent = atoms<Kind, tyr::formalism::FluentTag>(source);
    const auto target_fluent = atoms<Kind, tyr::formalism::FluentTag>(target);
    const auto source_derived = atoms<Kind, tyr::formalism::DerivedTag>(source);
    const auto target_derived = atoms<Kind, tyr::formalism::DerivedTag>(target);
    ASSERT_FALSE((atoms<Kind, tyr::formalism::StaticTag>(source).empty()));
    ASSERT_FALSE(source_derived.empty());
    ASSERT_LT(target_derived.size(), source_derived.size());
    check_state_delta<kr::BaseFamilyTag, Kind>(source, target);
    check_state_delta<kr::UnsFamilyTag, Kind>(source, target);

    auto denotations = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto source_registers = ygg::Data<sem::RegisterValues> {};
    source_registers.concept_values.resize(3);
    source_registers.concept_values[0] = Object(0);
    source_registers.concept_values[1] = Object(1);
    source_registers.role_values.resize(3);
    source_registers.role_values[0] = cista::pair(Object(0), Object(1));
    source_registers.role_values[1] = cista::pair(Object(1), Object(2));
    source_registers.role_values[2] = cista::pair(Object(0), Object(2));
    auto target_registers = ygg::Data<sem::RegisterValues> {};
    target_registers.concept_values.resize(4);
    target_registers.concept_values[0] = Object(1);
    target_registers.concept_values[2] = Object(2);
    target_registers.concept_values[3] = Object(0);
    target_registers.role_values.resize(2);
    target_registers.role_values[0] = source_registers.role_values[0];
    target_registers.role_values[1] = cista::pair(Object(2), Object(0));
    const auto source_register_view = ygg::make_view(source_registers, *search->task->get_repository());
    const auto target_register_view = ygg::make_view(target_registers, *search->task->get_repository());

    auto delta = Delta {};
    const auto registered_registers = denotations.insert(source_registers).first;
    delta.template assign<Kind>(source, registered_registers, source, registered_registers);
    EXPECT_TRUE(delta.empty());

    // Copied builders exercise the borrowed state path used by NONE and CHOICE search.
    const auto source_builder = source.get_state_builder();
    const auto target_builder = target.get_state_builder();
    const auto borrowed_source = ygg::make_view(source_builder, *search->task);
    const auto borrowed_target = ygg::make_view(target_builder, *search->task);
    const auto assign = [&] { delta.template assign<Kind>(borrowed_source, source_register_view, borrowed_target, target_register_view); };
    assign();
    EXPECT_FALSE(delta.empty());
    EXPECT_TRUE(std::ranges::is_permutation(delta.removed.fluent_atoms, difference(source_fluent, target_fluent)));
    EXPECT_TRUE(std::ranges::is_permutation(delta.added.fluent_atoms, difference(target_fluent, source_fluent)));
    EXPECT_TRUE(std::ranges::is_permutation(delta.removed.derived_atoms, difference(source_derived, target_derived)));
    EXPECT_TRUE(std::ranges::is_permutation(delta.added.derived_atoms, difference(target_derived, source_derived)));
    const auto object = [&](ygg::uint_t index) { return ygg::make_view(Object(index), *search->task->get_repository()); };
    const auto removed_concepts = std::vector { std::pair(ConceptRegister(0), object(0)), std::pair(ConceptRegister(1), object(1)) };
    const auto added_concepts =
        std::vector { std::pair(ConceptRegister(0), object(1)), std::pair(ConceptRegister(2), object(2)), std::pair(ConceptRegister(3), object(0)) };
    const auto removed_roles =
        std::vector { std::pair(RoleRegister(1), std::pair(object(1), object(2))), std::pair(RoleRegister(2), std::pair(object(0), object(2))) };
    const auto added_roles = std::vector { std::pair(RoleRegister(1), std::pair(object(2), object(0))) };
    EXPECT_EQ(delta.removed.concept_registers, removed_concepts);
    EXPECT_EQ(delta.added.concept_registers, added_concepts);
    EXPECT_EQ(delta.removed.role_registers, removed_roles);
    EXPECT_EQ(delta.added.role_registers, added_roles);

    const auto original = delta;
    auto inverse = Delta {};
    inverse.template assign<Kind>(target, target_register_view, source, source_register_view);
    delta.reverse();
    expect_same_side(delta.added, inverse.added);
    expect_same_side(delta.removed, inverse.removed);
    delta.reverse();
    expect_same_side(delta.added, original.added);
    expect_same_side(delta.removed, original.removed);

    const auto reserved = capacities(delta);
    delta.clear();
    EXPECT_TRUE(delta.empty());
    EXPECT_EQ(capacities(delta), reserved);
    assign();
    EXPECT_EQ(capacities(delta), reserved);
    expect_same_side(delta.added, original.added);
    expect_same_side(delta.removed, original.removed);

    // Adding unassigned slots changes the register layout without changing feature inputs.
    auto resized_registers = source_registers;
    resized_registers.concept_values.resize(4);
    resized_registers.role_values.resize(4);
    delta.template assign<Kind>(source, source_register_view, source, ygg::make_view(resized_registers, *search->task->get_repository()));
    EXPECT_TRUE(delta.empty());
    EXPECT_TRUE(delta.added.concept_registers.empty());
    EXPECT_TRUE(delta.removed.concept_registers.empty());
    EXPECT_TRUE(delta.added.role_registers.empty());
    EXPECT_TRUE(delta.removed.role_registers.empty());
    EXPECT_TRUE(delta.added.fluent_atoms.empty());
    EXPECT_TRUE(delta.removed.fluent_atoms.empty());
    EXPECT_TRUE(delta.added.derived_atoms.empty());
    EXPECT_TRUE(delta.removed.derived_atoms.empty());

    // Delta values snapshot repository objects, not proxies into mutable register slots.
    assign();
    source_registers.concept_values[0] = Object(2);
    source_registers.role_values[1] = cista::pair(Object(0), Object(0));
    target_registers.concept_values[0] = Object(2);
    target_registers.role_values[1] = cista::pair(Object(1), Object(1));
    EXPECT_EQ(delta.removed.concept_registers, removed_concepts);
    EXPECT_EQ(delta.added.concept_registers, added_concepts);
    EXPECT_EQ(delta.removed.role_registers, removed_roles);
    EXPECT_EQ(delta.added.role_registers, added_roles);
}
}  // namespace

TEST(RunirKrDlSemanticsDelta, GroundInputsAreReversibleAndReuseStorage) { check_delta<tyr::GroundTag>(); }
TEST(RunirKrDlSemanticsDelta, LiftedInputsAreReversibleAndReuseStorage) { check_delta<tyr::LiftedTag>(); }

}  // namespace runir::tests
