#ifndef RUNIR_SEMANTICS_EVALUATION_HPP_
#define RUNIR_SEMANTICS_EVALUATION_HPP_

#include "runir/common/config.hpp"
#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/semantics/denotations.hpp"
#include "runir/kr/dl/semantics/evaluation_context.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"

#include <cassert>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <tyr/common/dynamic_bitset.hpp>
#include <tyr/common/types.hpp>
#include <tyr/formalism/object_index.hpp>
#include <tyr/formalism/planning/ground_atom_view.hpp>
#include <tyr/formalism/planning/ground_conjunctive_condition_view.hpp>
#include <tyr/formalism/planning/ground_literal_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/ground_task.hpp>
#include <tyr/planning/lifted_task.hpp>
#include <utility>

namespace runir::kr::dl::semantics
{

template<CategoryTag Category, FamilyTag Family, tyr::planning::TaskKind Kind>
using EvaluationBuilderT = decltype(std::declval<EvaluationContext<Family, Kind>&>().get_builder().template get_builder<Denotation<Category>>());

template<FamilyTag Family, CategoryTag Category, tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyConstructor<Family, Category>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<Category, Family, Kind>;

template<FamilyTag Family, typename Tag, tyr::planning::TaskKind Kind, typename C>
    requires ConceptConstructorTag<Tag>
auto evaluate_impl(tyr::View<tyr::Index<FamilyConcept<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<ConceptTag, Family, Kind>;

template<FamilyTag Family, typename Tag, tyr::planning::TaskKind Kind, typename C>
    requires RoleConstructorTag<Tag>
auto evaluate_impl(tyr::View<tyr::Index<FamilyRole<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<RoleTag, Family, Kind>;

template<FamilyTag Family, BooleanConstructorTag Tag, tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyBoolean<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<BooleanTag, Family, Kind>;

template<FamilyTag Family, NumericalConstructorTag Tag, tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyNumerical<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<NumericalTag, Family, Kind>;

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyConcept<ExtFamilyTag, RegisterTag>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<ConceptTag, ExtFamilyTag, Kind>;

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyRole<ExtFamilyTag, RegisterTag>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<RoleTag, ExtFamilyTag, Kind>;

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyConcept<ExtFamilyTag, ArgumentTag<ConceptTag>>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<ConceptTag, ExtFamilyTag, Kind>;

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyRole<ExtFamilyTag, ArgumentTag<RoleTag>>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<RoleTag, ExtFamilyTag, Kind>;

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyBoolean<ExtFamilyTag, ArgumentTag<BooleanTag>>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<BooleanTag, ExtFamilyTag, Kind>;

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyNumerical<ExtFamilyTag, ArgumentTag<NumericalTag>>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<NumericalTag, ExtFamilyTag, Kind>;

namespace detail
{

template<FamilyTag Family, tyr::planning::TaskKind Kind>
auto num_objects(const EvaluationContext<Family, Kind>& context) noexcept -> uint_t
{
    const auto task = context.get_state().get_state_repository()->get_task()->get_task();
    return static_cast<uint_t>(task.get_domain().get_constants().size() + task.get_objects().size());
}

template<FamilyTag Family, tyr::planning::TaskKind Kind>
auto make_concept_builder(EvaluationContext<Family, Kind>& context)
{
    return context.get_builder().template get_builder<Denotation<ConceptTag>>(num_objects(context));
}

template<FamilyTag Family, tyr::planning::TaskKind Kind>
auto make_role_builder(EvaluationContext<Family, Kind>& context)
{
    return context.get_builder().template get_builder<Denotation<RoleTag>>(num_objects(context));
}

inline auto object_handle(uint_t object) noexcept { return tyr::Index<tyr::formalism::Object>(object); }

template<typename T, typename C, typename Context>
decltype(auto) constructor_child_view(const tyr::View<T, C>& child, const Context&) noexcept
{
    return child;
}

template<typename T, typename C>
auto constructor_child_view(T child, const C& context) noexcept
{
    return tyr::make_view(child, context);
}

template<typename RoleBuilderPtr>
auto row(const RoleBuilderPtr& role, uint_t object) noexcept
{
    return role->get_row_bitset(object_handle(object));
}

template<typename RoleBuilderPtr>
auto row(const RoleBuilderPtr& role, tyr::Index<tyr::formalism::Object> object) noexcept
{
    return role->get_row_bitset(object);
}

template<typename RoleBuilderPtr>
bool role_any(const RoleBuilderPtr& role) noexcept
{
    const auto num_objects_ = role->num_objects;
    for (uint_t object = 0; object < num_objects_; ++object)
        if (row(role, object).any())
            return true;

    return false;
}

template<typename RoleBuilderPtr>
auto role_count(const RoleBuilderPtr& role) noexcept -> uint_t
{
    const auto num_objects_ = role->num_objects;
    uint_t result = 0;
    for (uint_t object = 0; object < num_objects_; ++object)
        result += static_cast<uint_t>(row(role, object).count());

    return result;
}

template<FamilyTag Family, tyr::planning::TaskKind Kind, typename F>
void for_each_current_atom(EvaluationContext<Family, Kind>& context, std::type_identity<tyr::formalism::StaticTag>, F&& f)
{
    for (auto atom : context.get_state().get_static_atoms_view())
        std::forward<F>(f)(atom);
}

template<FamilyTag Family, tyr::planning::TaskKind Kind, typename F>
void for_each_current_atom(EvaluationContext<Family, Kind>& context, std::type_identity<tyr::formalism::FluentTag>, F&& f)
{
    for (auto fact : context.get_state().get_fluent_facts_view())
        if (auto atom = fact.get_atom())
            std::forward<F>(f)(*atom);
}

template<FamilyTag Family, tyr::planning::TaskKind Kind, typename F>
void for_each_current_atom(EvaluationContext<Family, Kind>& context, std::type_identity<tyr::formalism::DerivedTag>, F&& f)
{
    for (auto atom : context.get_state().get_derived_atoms_view())
        std::forward<F>(f)(atom);
}

template<tyr::formalism::FactKind T, FamilyTag Family, tyr::planning::TaskKind Kind, typename F>
void for_each_current_atom(EvaluationContext<Family, Kind>& context, F&& f)
{
    for_each_current_atom(context, std::type_identity<T> {}, std::forward<F>(f));
}

template<tyr::formalism::FactKind T, typename Goal, typename F>
void for_each_goal_literal_atom(Goal goal, bool polarity, F&& f)
{
    for (auto literal : goal.template get_literals<T>())
        if (literal.get_polarity() == polarity)
            std::forward<F>(f)(literal.get_atom());
}

template<typename Goal, typename F>
void for_each_goal_fact_atom(Goal goal, tyr::formalism::PositiveTag, F&& f)
{
    for (auto fact : goal.template get_facts<tyr::formalism::PositiveTag>())
        if (auto atom = fact.get_atom())
            std::forward<F>(f)(*atom);
}

template<typename Goal, typename F>
void for_each_goal_fact_atom(Goal goal, tyr::formalism::NegativeTag, F&& f)
{
    for (auto fact : goal.template get_facts<tyr::formalism::NegativeTag>())
        if (auto atom = fact.get_atom())
            std::forward<F>(f)(*atom);
}

template<tyr::formalism::FactKind T, typename Goal, typename F>
void for_each_goal_atom(Goal goal, bool polarity, F&& f)
{
    if constexpr (std::same_as<T, tyr::formalism::FluentTag> && requires { goal.template get_facts<tyr::formalism::PositiveTag>(); })
    {
        if (polarity)
            for_each_goal_fact_atom(goal, tyr::formalism::PositiveTag {}, std::forward<F>(f));
        else
            for_each_goal_fact_atom(goal, tyr::formalism::NegativeTag {}, std::forward<F>(f));
    }
    else
    {
        for_each_goal_literal_atom<T>(goal, polarity, std::forward<F>(f));
    }
}

template<tyr::formalism::FactKind T, FamilyTag Family, tyr::planning::TaskKind Kind, typename F>
void for_each_goal_atom(EvaluationContext<Family, Kind>& context, bool polarity, F&& f)
{
    const auto goal = context.get_state().get_state_repository()->get_task()->get_task().get_goal();
    for_each_goal_atom<T>(goal, polarity, std::forward<F>(f));
}

template<tyr::formalism::FactKind T>
auto object_index(tyr::formalism::planning::GroundAtomView<T> atom, size_t position) noexcept -> tyr::Index<tyr::formalism::Object>
{
    return atom.get_row().get_objects()[position].get_index();
}

template<FamilyTag Family, tyr::formalism::FactKind T, tyr::planning::TaskKind Kind, typename C>
auto evaluate_atomic_state_concept(tyr::View<tyr::Index<FamilyConcept<Family, AtomicStateTag<T>>>, C> constructor, EvaluationContext<Family, Kind>& context)
{
    [[maybe_unused]] const auto num_objects = detail::num_objects(context);
    auto result = detail::make_concept_builder(context);
    auto bitset = result->get_bitset();

    detail::for_each_current_atom<T>(context,
                                     [&](auto atom)
                                     {
                                         if (atom.get_predicate().get_index() != constructor.get_data().predicate)
                                             return;

                                         const auto object = detail::object_index(atom, 0);
                                         assert(tyr::uint_t(object) < num_objects);
                                         bitset.set(tyr::uint_t(object));
                                     });

    if (!constructor.get_polarity())
        bitset.flip();

    return result;
}

template<FamilyTag Family, tyr::formalism::FactKind T, tyr::planning::TaskKind Kind, typename C>
auto evaluate_atomic_goal_concept(tyr::View<tyr::Index<FamilyConcept<Family, AtomicGoalTag<T>>>, C> constructor, EvaluationContext<Family, Kind>& context)
{
    [[maybe_unused]] const auto num_objects = detail::num_objects(context);
    auto result = detail::make_concept_builder(context);
    auto bitset = result->get_bitset();

    detail::for_each_goal_atom<T>(context,
                                  constructor.get_polarity(),
                                  [&](auto atom)
                                  {
                                      if (atom.get_predicate().get_index() != constructor.get_data().predicate)
                                          return;

                                      const auto object = detail::object_index(atom, 0);
                                      assert(tyr::uint_t(object) < num_objects);
                                      bitset.set(tyr::uint_t(object));
                                  });

    return result;
}

template<FamilyTag Family, tyr::formalism::FactKind T, tyr::planning::TaskKind Kind, typename C>
auto evaluate_atomic_state_role(tyr::View<tyr::Index<FamilyRole<Family, AtomicStateTag<T>>>, C> constructor, EvaluationContext<Family, Kind>& context)
{
    [[maybe_unused]] const auto num_objects = detail::num_objects(context);
    auto result = detail::make_role_builder(context);

    detail::for_each_current_atom<T>(context,
                                     [&](auto atom)
                                     {
                                         if (atom.get_predicate().get_index() != constructor.get_data().predicate)
                                             return;

                                         const auto lhs = detail::object_index(atom, 0);
                                         const auto rhs = detail::object_index(atom, 1);
                                         assert(tyr::uint_t(lhs) < num_objects);
                                         assert(tyr::uint_t(rhs) < num_objects);
                                         detail::row(result, lhs).set(tyr::uint_t(rhs));
                                     });

    if (!constructor.get_polarity())
        for (uint_t object = 0; object < num_objects; ++object)
            detail::row(result, object).flip();

    return result;
}

template<FamilyTag Family, tyr::formalism::FactKind T, tyr::planning::TaskKind Kind, typename C>
auto evaluate_atomic_goal_role(tyr::View<tyr::Index<FamilyRole<Family, AtomicGoalTag<T>>>, C> constructor, EvaluationContext<Family, Kind>& context)
{
    [[maybe_unused]] const auto num_objects = detail::num_objects(context);
    auto result = detail::make_role_builder(context);

    detail::for_each_goal_atom<T>(context,
                                  constructor.get_polarity(),
                                  [&](auto atom)
                                  {
                                      if (atom.get_predicate().get_index() != constructor.get_data().predicate)
                                          return;

                                      const auto lhs = detail::object_index(atom, 0);
                                      const auto rhs = detail::object_index(atom, 1);
                                      assert(tyr::uint_t(lhs) < num_objects);
                                      assert(tyr::uint_t(rhs) < num_objects);
                                      detail::row(result, lhs).set(tyr::uint_t(rhs));
                                  });

    return result;
}

template<FamilyTag Family, tyr::formalism::FactKind T, tyr::planning::TaskKind Kind, typename C>
auto evaluate_atomic_state_boolean(tyr::View<tyr::Index<FamilyBoolean<Family, AtomicStateTag<T>>>, C> constructor, EvaluationContext<Family, Kind>& context)
{
    bool value = false;

    detail::for_each_current_atom<T>(context,
                                     [&](auto atom)
                                     {
                                         if (atom.get_predicate().get_index() == constructor.get_data().predicate)
                                             value = true;
                                     });

    if (!constructor.get_polarity())
        value = !value;

    return context.get_builder().template get_builder<Denotation<BooleanTag>>(value);
}

template<FamilyTag Family, tyr::formalism::FactKind T, tyr::planning::TaskKind Kind, typename C>
auto evaluate_atomic_goal_boolean(tyr::View<tyr::Index<FamilyBoolean<Family, AtomicGoalTag<T>>>, C> constructor, EvaluationContext<Family, Kind>& context)
{
    bool value = false;

    detail::for_each_goal_atom<T>(context,
                                  constructor.get_polarity(),
                                  [&](auto atom)
                                  {
                                      if (atom.get_predicate().get_index() == constructor.get_data().predicate)
                                          value = true;
                                  });

    return context.get_builder().template get_builder<Denotation<BooleanTag>>(value);
}

template<FamilyTag Family, tyr::planning::TaskKind Kind, typename C>
bool evaluate_nonempty(tyr::View<tyr::Index<FamilyConstructor<Family, ConceptTag>>, C> constructor,
                       EvaluationContext<Family, Kind>& context,
                       EvaluationWorkspace& workspace)
{
    return evaluate_impl(constructor, context, workspace)->get_bitset().any();
}

template<FamilyTag Family, tyr::planning::TaskKind Kind, typename C>
bool evaluate_nonempty(tyr::View<tyr::Index<FamilyConstructor<Family, RoleTag>>, C> constructor,
                       EvaluationContext<Family, Kind>& context,
                       EvaluationWorkspace& workspace)
{
    return detail::role_any(evaluate_impl(constructor, context, workspace));
}

template<FamilyTag Family, tyr::planning::TaskKind Kind, typename C>
auto evaluate_count(tyr::View<tyr::Index<FamilyConstructor<Family, ConceptTag>>, C> constructor,
                    EvaluationContext<Family, Kind>& context,
                    EvaluationWorkspace& workspace) -> uint_t
{
    return static_cast<uint_t>(evaluate_impl(constructor, context, workspace)->get_bitset().count());
}

template<FamilyTag Family, tyr::planning::TaskKind Kind, typename C>
auto evaluate_count(tyr::View<tyr::Index<FamilyConstructor<Family, RoleTag>>, C> constructor,
                    EvaluationContext<Family, Kind>& context,
                    EvaluationWorkspace& workspace) -> uint_t
{
    return detail::role_count(evaluate_impl(constructor, context, workspace));
}

}

template<FamilyTag Family, typename Tag, tyr::planning::TaskKind Kind, typename C>
    requires ConceptConstructorTag<Tag>
auto evaluate_impl(tyr::View<tyr::Index<FamilyConcept<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<ConceptTag, Family, Kind>
{
    [[maybe_unused]] const auto num_objects = detail::num_objects(context);
    auto result = detail::make_concept_builder(context);
    auto result_bitset = result->get_bitset();

    if constexpr (std::same_as<Tag, BotTag>) {}
    else if constexpr (std::same_as<Tag, TopTag>)
    {
        result_bitset.set();
    }
    else if constexpr (is_atomic_state_tag_v<Tag>)
    {
        return detail::evaluate_atomic_state_concept(constructor, context);
    }
    else if constexpr (is_atomic_goal_tag_v<Tag>)
    {
        return detail::evaluate_atomic_goal_concept(constructor, context);
    }
    else if constexpr (std::same_as<Tag, IntersectionTag>)
    {
        const auto lhs = evaluate_impl(constructor.get_lhs(), context, workspace);
        const auto rhs = evaluate_impl(constructor.get_rhs(), context, workspace);
        result_bitset.copy_from(lhs->get_bitset());
        result_bitset &= rhs->get_bitset();
    }
    else if constexpr (std::same_as<Tag, UnionTag>)
    {
        const auto lhs = evaluate_impl(constructor.get_lhs(), context, workspace);
        const auto rhs = evaluate_impl(constructor.get_rhs(), context, workspace);
        result_bitset.copy_from(lhs->get_bitset());
        result_bitset |= rhs->get_bitset();
    }
    else if constexpr (std::same_as<Tag, NegationTag>)
    {
        const auto arg = evaluate_impl(constructor.get_arg(), context, workspace);
        result_bitset.copy_from(arg->get_bitset());
        result_bitset.flip();
    }
    else if constexpr (std::same_as<Tag, ValueRestrictionTag>)
    {
        const auto role = evaluate_impl(constructor.get_lhs(), context, workspace);
        const auto concept_denotation = evaluate_impl(constructor.get_rhs(), context, workspace);
        const auto concept_bitset = concept_denotation->get_bitset();

        result_bitset.set();
        for (uint_t object = 0; object < num_objects; ++object)
        {
            if (!detail::row(role, object).is_subset_of(concept_bitset))
                result_bitset.reset(object);
        }
    }
    else if constexpr (std::same_as<Tag, ExistentialQuantificationTag>)
    {
        const auto role = evaluate_impl(constructor.get_lhs(), context, workspace);
        const auto concept_denotation = evaluate_impl(constructor.get_rhs(), context, workspace);
        const auto concept_bitset = concept_denotation->get_bitset();

        for (uint_t object = 0; object < num_objects; ++object)
        {
            if (detail::row(role, object).intersects(concept_bitset))
                result_bitset.set(object);
        }
    }
    else if constexpr (std::same_as<Tag, AtLeastNumberRestrictionTag> || std::same_as<Tag, AtMostNumberRestrictionTag>
                       || std::same_as<Tag, ExactNumberRestrictionTag>)
    {
        const auto role = evaluate_impl(constructor.get_role(), context, workspace);
        for (uint_t object = 0; object < num_objects; ++object)
        {
            const auto count = static_cast<uint_t>(detail::row(role, object).count());
            if constexpr (std::same_as<Tag, AtLeastNumberRestrictionTag>)
            {
                if (count >= constructor.get_n())
                    result_bitset.set(object);
            }
            else if constexpr (std::same_as<Tag, AtMostNumberRestrictionTag>)
            {
                if (count <= constructor.get_n())
                    result_bitset.set(object);
            }
            else if constexpr (std::same_as<Tag, ExactNumberRestrictionTag>)
            {
                if (count == constructor.get_n())
                    result_bitset.set(object);
            }
        }
    }
    else if constexpr (std::same_as<Tag, QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, QualifiedAtMostNumberRestrictionTag>
                       || std::same_as<Tag, QualifiedExactNumberRestrictionTag>)
    {
        const auto role = evaluate_impl(constructor.get_role(), context, workspace);
        const auto concept_denotation = evaluate_impl(constructor.get_concept(), context, workspace);
        const auto concept_bitset = concept_denotation->get_bitset();

        for (uint_t object = 0; object < num_objects; ++object)
        {
            auto count = uint_t { 0 };
            auto row = detail::row(role, object);
            auto target = row.find_first();
            while (target != decltype(row)::npos)
            {
                if (concept_bitset.test(target))
                    ++count;
                target = row.find_next(target);
            }
            if constexpr (std::same_as<Tag, QualifiedAtLeastNumberRestrictionTag>)
            {
                if (count >= constructor.get_n())
                    result_bitset.set(object);
            }
            else if constexpr (std::same_as<Tag, QualifiedAtMostNumberRestrictionTag>)
            {
                if (count <= constructor.get_n())
                    result_bitset.set(object);
            }
            else if constexpr (std::same_as<Tag, QualifiedExactNumberRestrictionTag>)
            {
                if (count == constructor.get_n())
                    result_bitset.set(object);
            }
        }
    }
    else if constexpr (std::same_as<Tag, RoleValueMapTag>)
    {
        const auto lhs = evaluate_impl(constructor.get_lhs(), context, workspace);
        const auto rhs = evaluate_impl(constructor.get_rhs(), context, workspace);

        result_bitset.set();
        for (uint_t object = 0; object < num_objects; ++object)
        {
            if (!detail::row(lhs, object).is_subset_of(detail::row(rhs, object)))
                result_bitset.reset(object);
        }
    }
    else if constexpr (std::same_as<Tag, AgreementTag>)
    {
        const auto lhs = evaluate_impl(constructor.get_lhs(), context, workspace);
        const auto rhs = evaluate_impl(constructor.get_rhs(), context, workspace);

        result_bitset.set();
        for (uint_t object = 0; object < num_objects; ++object)
        {
            if (detail::row(lhs, object) != detail::row(rhs, object))
                result_bitset.reset(object);
        }
    }
    else if constexpr (std::same_as<Tag, NominalTag>)
    {
        const auto object = constructor.get_object().get_index();
        assert(tyr::uint_t(object) < num_objects);
        result_bitset.set(tyr::uint_t(object));
    }
    else if constexpr (std::same_as<Tag, RoleFillersTag>)
    {
        const auto role = evaluate_impl(constructor.get_role(), context, workspace);
        for (uint_t object = 0; object < num_objects; ++object)
        {
            auto row = detail::row(role, object);
            auto contains_all_fillers = true;
            for (auto filler : constructor.get_objects())
            {
                if (!row.test(tyr::uint_t(filler.get_index())))
                {
                    contains_all_fillers = false;
                    break;
                }
            }
            if (contains_all_fillers)
                result_bitset.set(object);
        }
    }
    else if constexpr (std::same_as<Tag, OneOfTag>)
    {
        for (auto object : constructor.get_objects())
            result_bitset.set(tyr::uint_t(object.get_index()));
    }
    else if constexpr (std::same_as<Tag, RegisterTag>)
    {
        static_assert(!std::same_as<Tag, RegisterTag>, "Register concept evaluation is defined by the ext semantics evaluator.");
    }
    else if constexpr (std::same_as<Tag, ArgumentTag<ConceptTag>>)
    {
        static_assert(!std::same_as<Tag, ArgumentTag<ConceptTag>>, "Argument concept evaluation is defined by the ext semantics evaluator.");
    }

    return result;
}

template<FamilyTag Family, typename Tag, tyr::planning::TaskKind Kind, typename C>
    requires RoleConstructorTag<Tag>
auto evaluate_impl(tyr::View<tyr::Index<FamilyRole<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<RoleTag, Family, Kind>
{
    [[maybe_unused]] const auto num_objects = detail::num_objects(context);
    auto result = detail::make_role_builder(context);

    if constexpr (std::same_as<Tag, UniversalTag>)
    {
        for (uint_t object = 0; object < num_objects; ++object)
            detail::row(result, object).set();
    }
    else if constexpr (is_atomic_state_tag_v<Tag>)
    {
        return detail::evaluate_atomic_state_role(constructor, context);
    }
    else if constexpr (is_atomic_goal_tag_v<Tag>)
    {
        return detail::evaluate_atomic_goal_role(constructor, context);
    }
    else if constexpr (std::same_as<Tag, IntersectionTag>)
    {
        const auto lhs = evaluate_impl(constructor.get_lhs(), context, workspace);
        const auto rhs = evaluate_impl(constructor.get_rhs(), context, workspace);

        for (uint_t object = 0; object < num_objects; ++object)
        {
            auto row = detail::row(result, object);
            row.copy_from(detail::row(lhs, object));
            row &= detail::row(rhs, object);
        }
    }
    else if constexpr (std::same_as<Tag, UnionTag>)
    {
        const auto lhs = evaluate_impl(constructor.get_lhs(), context, workspace);
        const auto rhs = evaluate_impl(constructor.get_rhs(), context, workspace);

        for (uint_t object = 0; object < num_objects; ++object)
        {
            auto row = detail::row(result, object);
            row.copy_from(detail::row(lhs, object));
            row |= detail::row(rhs, object);
        }
    }
    else if constexpr (std::same_as<Tag, ComplementTag>)
    {
        const auto arg = evaluate_impl(constructor.get_arg(), context, workspace);

        for (uint_t object = 0; object < num_objects; ++object)
        {
            auto row = detail::row(result, object);
            row.copy_from(detail::row(arg, object));
            row.flip();
        }
    }
    else if constexpr (std::same_as<Tag, InverseTag>)
    {
        const auto arg = evaluate_impl(constructor.get_arg(), context, workspace);

        for (uint_t lhs = 0; lhs < num_objects; ++lhs)
        {
            const auto row = detail::row(arg, lhs);
            for (auto rhs = row.find_first(); rhs != decltype(row)::npos; rhs = row.find_next(rhs))
                detail::row(result, static_cast<uint_t>(rhs)).set(lhs);
        }
    }
    else if constexpr (std::same_as<Tag, CompositionTag>)
    {
        const auto lhs = evaluate_impl(constructor.get_lhs(), context, workspace);
        const auto rhs = evaluate_impl(constructor.get_rhs(), context, workspace);

        for (uint_t source = 0; source < num_objects; ++source)
        {
            auto result_row = detail::row(result, source);
            const auto lhs_row = detail::row(lhs, source);

            for (auto mid = lhs_row.find_first(); mid != decltype(lhs_row)::npos; mid = lhs_row.find_next(mid))
                result_row |= detail::row(rhs, static_cast<uint_t>(mid));
        }
    }
    else if constexpr (std::same_as<Tag, TransitiveClosureTag> || std::same_as<Tag, ReflexiveTransitiveClosureTag>)
    {
        const auto arg = evaluate_impl(constructor.get_arg(), context, workspace);

        for (uint_t object = 0; object < num_objects; ++object)
            detail::row(result, object).copy_from(detail::row(arg, object));

        for (uint_t mid = 0; mid < num_objects; ++mid)
        {
            for (uint_t source = 0; source < num_objects; ++source)
            {
                auto source_row = detail::row(result, source);
                if (source_row[mid])
                    source_row |= detail::row(result, mid);
            }
        }

        if constexpr (std::same_as<Tag, ReflexiveTransitiveClosureTag>)
            for (uint_t object = 0; object < num_objects; ++object)
                detail::row(result, object).set(object);
    }
    else if constexpr (std::same_as<Tag, RestrictionTag>)
    {
        const auto role = evaluate_impl(constructor.get_lhs(), context, workspace);
        const auto concept_denotation = evaluate_impl(constructor.get_rhs(), context, workspace);
        const auto concept_bitset = concept_denotation->get_bitset();

        for (uint_t object = 0; object < num_objects; ++object)
        {
            auto row = detail::row(result, object);
            row.copy_from(detail::row(role, object));
            row &= concept_bitset;
        }
    }
    else if constexpr (std::same_as<Tag, IdentityTag>)
    {
        const auto concept_denotation = evaluate_impl(constructor.get_arg(), context, workspace);
        const auto bitset = concept_denotation->get_bitset();

        for (auto object = bitset.find_first(); object != decltype(bitset)::npos; object = bitset.find_next(object))
            detail::row(result, static_cast<uint_t>(object)).set(object);
    }
    else if constexpr (std::same_as<Tag, RegisterTag>)
    {
        static_assert(!std::same_as<Tag, RegisterTag>, "Register role evaluation is defined by the ext semantics evaluator.");
    }
    else if constexpr (std::same_as<Tag, ArgumentTag<RoleTag>>)
    {
        static_assert(!std::same_as<Tag, ArgumentTag<RoleTag>>, "Argument role evaluation is defined by the ext semantics evaluator.");
    }

    return result;
}

template<FamilyTag Family, BooleanConstructorTag Tag, tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyBoolean<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<BooleanTag, Family, Kind>
{
    if constexpr (is_atomic_state_tag_v<Tag>)
    {
        return detail::evaluate_atomic_state_boolean(constructor, context);
    }
    else if constexpr (is_atomic_goal_tag_v<Tag>)
    {
        return detail::evaluate_atomic_goal_boolean(constructor, context);
    }
    else if constexpr (std::same_as<Tag, NonemptyTag>)
    {
        const auto result_value = tyr::visit([&](auto arg) { return detail::evaluate_nonempty(arg, context, workspace); }, constructor.get_arg());
        return context.get_builder().template get_builder<Denotation<BooleanTag>>(result_value);
    }
    else if constexpr (std::same_as<Tag, ArgumentTag<BooleanTag>>)
    {
        static_assert(!std::same_as<Tag, ArgumentTag<BooleanTag>>, "Argument boolean evaluation is defined by the ext semantics evaluator.");
    }
}

template<FamilyTag Family, NumericalConstructorTag Tag, tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyNumerical<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<NumericalTag, Family, Kind>
{
    uint_t result_value = 0;

    if constexpr (std::same_as<Tag, CountTag>)
    {
        result_value = tyr::visit([&](auto arg) { return detail::evaluate_count(arg, context, workspace); }, constructor.get_arg());
    }
    else if constexpr (std::same_as<Tag, ArgumentTag<NumericalTag>>)
    {
        static_assert(!std::same_as<Tag, ArgumentTag<NumericalTag>>, "Argument numerical evaluation is defined by the ext semantics evaluator.");
    }
    else if constexpr (std::same_as<Tag, DistanceTag>)
    {
        const auto lhs = evaluate_impl(constructor.get_lhs(), context, workspace);
        const auto role = evaluate_impl(constructor.get_mid(), context, workspace);
        const auto rhs = evaluate_impl(constructor.get_rhs(), context, workspace);
        [[maybe_unused]] const auto num_objects = detail::num_objects(context);

        result_value = std::numeric_limits<uint_t>::max();

        const auto lhs_bitset = lhs->get_bitset();
        const auto rhs_bitset = rhs->get_bitset();

        if (lhs_bitset.any() && rhs_bitset.any())
        {
            if (lhs_bitset.intersects(rhs_bitset))
            {
                result_value = 0;
            }
            else
            {
                workspace.prepare_distance(num_objects);
                auto& queue = workspace.get_distance_queue();
                auto& distances = workspace.get_distance_values();
                size_t queue_pos = 0;

                for (auto object = lhs_bitset.find_first(); object != decltype(lhs_bitset)::npos; object = lhs_bitset.find_next(object))
                {
                    queue.push_back(static_cast<uint_t>(object));
                    distances[object] = 0;
                }

                while (queue_pos < queue.size())
                {
                    const auto source = queue[queue_pos++];

                    const auto source_distance = distances[source];
                    assert(source_distance != std::numeric_limits<uint_t>::max());

                    const auto row = detail::row(role, source);
                    for (auto target = row.find_first(); target != decltype(row)::npos; target = row.find_next(target))
                    {
                        const auto new_distance = source_distance + 1;
                        auto& target_distance = distances[target];

                        if (new_distance < target_distance)
                        {
                            target_distance = new_distance;
                            queue.push_back(static_cast<uint_t>(target));
                        }

                        if (rhs_bitset[target])
                        {
                            result_value = target_distance;
                            queue_pos = queue.size();
                            break;
                        }
                    }
                }
            }
        }
    }

    auto result = context.get_builder().template get_builder<Denotation<NumericalTag>>(result_value);
    return result;
}

template<FamilyTag Family, CategoryTag Category, tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyConstructor<Family, Category>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> EvaluationBuilderT<Category, Family, Kind>
{
    return tyr::visit([&](auto child) { return evaluate_impl(detail::constructor_child_view(child, constructor.get_context()), context, workspace); },
                      constructor.get_variant());
}

template<FamilyTag Family, CategoryTag Category, tyr::planning::TaskKind Kind, typename C>
auto evaluate(tyr::View<tyr::Index<FamilyConstructor<Family, Category>>, C> constructor,
              EvaluationContext<Family, Kind>& context,
              EvaluationWorkspace& workspace)
{
    auto result = evaluate_impl(constructor, context, workspace);
    return context.get_denotation_repository().get_or_create(*result).first;
}

template<FamilyTag Family, CategoryTag Category, tyr::planning::TaskKind Kind, typename C>
auto evaluate(tyr::View<tyr::Index<FamilyConstructor<Family, Category>>, C> constructor, EvaluationContext<Family, Kind>& context)
{
    auto workspace = EvaluationWorkspace {};
    return evaluate(constructor, context, workspace);
}

}

#endif
