#ifndef RUNIR_SEMANTICS_EVALUATION_HPP_
#define RUNIR_SEMANTICS_EVALUATION_HPP_

#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotations.hpp"
#include "runir/kr/dl/semantics/evaluation_context.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"

#include <cassert>
#include <concepts>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <tyr/formalism/object_index.hpp>
#include <tyr/formalism/planning/ground_conjunctive_condition_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/ground/state_repository.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/state_repository.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <utility>
#include <yggdrasil/containers/dynamic_bitset.hpp>
#include <yggdrasil/containers/unique_object_pool.hpp>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::semantics
{

template<CategoryTag Category, FamilyTag Family>
auto with_cache(FamilyConstructorView<Family, Category> constructor,
                DenotationCaches<Family>& caches,
                std::invocable auto&& evaluate) -> DenotationView<Category>
{
    auto& cache = caches.template get<Category>();
    if (const auto it = cache.find(constructor); it != cache.end())
        return it->second;

    return cache.emplace(constructor, std::forward<decltype(evaluate)>(evaluate)()).first->second;
}

template<FamilyTag Family, CategoryTag Category, tyr::TaskKind Kind, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConstructor<Family, Category>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<Category>>>;

template<FamilyTag Family, CategoryTag Category, tyr::TaskKind Kind>
auto evaluate_impl(FamilyConstructorView<Family, Category> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace,
                   DenotationCaches<Family>& caches) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<Category>>>;

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyConceptConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConcept<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<ConceptTag>>>;

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyConceptConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConcept<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace,
                   DenotationCaches<Family>& caches) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<ConceptTag>>>;

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyRoleConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyRole<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<RoleTag>>>;

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyRoleConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyRole<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace,
                   DenotationCaches<Family>& caches) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<RoleTag>>>;

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyBooleanConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyBoolean<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<BooleanTag>>>;

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyBooleanConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyBoolean<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace,
                   DenotationCaches<Family>& caches) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<BooleanTag>>>;

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyNumericalConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyNumerical<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<NumericalTag>>>;

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyNumericalConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyNumerical<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace,
                   DenotationCaches<Family>& caches) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<NumericalTag>>>;

template<FamilyTag Family, CategoryTag Category, tyr::TaskKind Kind>
auto evaluate(FamilyConstructorView<Family, Category> constructor,
              EvaluationContext<Family, Kind>& context,
              EvaluationWorkspace& workspace,
              DenotationCaches<Family>& caches) -> DenotationView<Category>;

namespace detail
{

template<FamilyTag Family, tyr::TaskKind Kind>
auto num_objects(const EvaluationContext<Family, Kind>& context) noexcept -> ygg::uint_t
{
    const auto task = context.get_state().get_state_repository()->get_task()->get_task();
    return static_cast<ygg::uint_t>(task.get_domain().get_constants().size() + task.get_objects().size());
}

template<FamilyTag Family, tyr::TaskKind Kind>
auto make_concept_builder(EvaluationContext<Family, Kind>& context)
{
    return context.get_builder().template get_builder<Denotation<ConceptTag>>(num_objects(context));
}

template<FamilyTag Family, tyr::TaskKind Kind>
auto make_role_builder(EvaluationContext<Family, Kind>& context)
{
    return context.get_builder().template get_builder<Denotation<RoleTag>>(num_objects(context));
}

template<CategoryTag Category, FamilyTag Family, tyr::TaskKind Kind>
auto materialize_denotation(ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<Category>>>& result, EvaluationContext<Family, Kind>& context)
{
    auto data = runir::kr::dl::semantics::checkout<Denotation<Category>>(context.get_builder());
    make_data(*result, *data);

    auto& repository = context.get_denotation_repository();
    if constexpr (std::same_as<Category, ConceptTag> || std::same_as<Category, RoleTag>)
        data->vec_index = repository.get_vector_repository().insert(result->blocks);

    auto interned = get_or_create(repository, *data);
    result->index = interned.first.get_index();
    return interned;
}

template<CategoryTag Category>
const DenotationView<Category>& deref(const DenotationView<Category>& denotation) noexcept
{
    return denotation;
}

template<CategoryTag Category>
const ygg::Builder<Denotation<Category>>& deref(const ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<Category>>>& denotation) noexcept
{
    return *denotation;
}

template<FamilyTag Family, tyr::TaskKind Kind, typename F>
void for_each_current_atom(EvaluationContext<Family, Kind>& context, std::type_identity<tyr::formalism::StaticTag>, F&& f)
{
    for (auto atom : context.get_state().get_static_atoms_view())
        std::forward<F>(f)(atom);
}

template<FamilyTag Family, tyr::TaskKind Kind, typename F>
void for_each_current_atom(EvaluationContext<Family, Kind>& context, std::type_identity<tyr::formalism::FluentTag>, F&& f)
{
    for (auto fact : context.get_state().get_fluent_facts_view())
        if (auto atom = fact.get_atom())
            std::forward<F>(f)(*atom);
}

template<FamilyTag Family, tyr::TaskKind Kind, typename F>
void for_each_current_atom(EvaluationContext<Family, Kind>& context, std::type_identity<tyr::formalism::DerivedTag>, F&& f)
{
    for (auto atom : context.get_state().get_derived_atoms_view())
        std::forward<F>(f)(atom);
}

template<tyr::formalism::FactKind T, FamilyTag Family, tyr::TaskKind Kind, typename F>
void for_each_current_atom(EvaluationContext<Family, Kind>& context, F&& f)
{
    for_each_current_atom(context, std::type_identity<T> {}, std::forward<F>(f));
}

template<tyr::formalism::FactKind T, FamilyTag Family, tyr::TaskKind Kind>
void for_each_goal_atom(EvaluationContext<Family, Kind>& context, bool polarity, std::invocable<tyr::formalism::planning::GroundAtomView<T>> auto&& f)
{
    const auto goal = context.get_state().get_state_repository()->get_task()->get_task().get_goal();
    if constexpr (std::same_as<T, tyr::formalism::FluentTag>)
    {
        const auto facts = polarity ? goal.template get_facts<tyr::formalism::PositiveTag>() : goal.template get_facts<tyr::formalism::NegativeTag>();
        for (auto fact : facts)
            if (auto atom = fact.get_atom())
                f(*atom);
    }
    else
    {
        for (auto literal : goal.template get_literals<T>())
            if (literal.get_polarity() == polarity)
                f(literal.get_atom());
    }
}

template<tyr::formalism::FactKind T>
auto object_index(tyr::formalism::planning::GroundAtomView<T> atom, size_t position) noexcept -> ygg::Index<tyr::formalism::Object>
{
    return atom.get_row().get_objects()[position].get_index();
}

template<FamilyTag Family, tyr::formalism::FactKind T, tyr::TaskKind Kind, typename C>
auto evaluate_atomic_state_concept(ygg::View<ygg::Index<FamilyConcept<Family, AtomicStateTag<T>>>, C> constructor, EvaluationContext<Family, Kind>& context)
{
    [[maybe_unused]] const auto num_objects = detail::num_objects(context);
    auto result = detail::make_concept_builder(context);
    auto bitset = result->get();

    detail::for_each_current_atom<T>(context,
                                     [&](auto atom)
                                     {
                                         if (atom.get_predicate().get_index() != constructor.get_data().predicate)
                                             return;

                                         const auto object = detail::object_index(atom, 0);
                                         assert(ygg::uint_t(object) < num_objects);
                                         bitset.set(ygg::uint_t(object));
                                     });

    if (!constructor.get_polarity())
        bitset.flip();

    return result;
}

template<FamilyTag Family, tyr::formalism::FactKind T, tyr::TaskKind Kind, typename C>
auto evaluate_atomic_goal_concept(ygg::View<ygg::Index<FamilyConcept<Family, AtomicGoalTag<T>>>, C> constructor, EvaluationContext<Family, Kind>& context)
{
    [[maybe_unused]] const auto num_objects = detail::num_objects(context);
    auto result = detail::make_concept_builder(context);
    auto bitset = result->get();

    detail::for_each_goal_atom<T>(context,
                                  constructor.get_polarity(),
                                  [&](auto atom)
                                  {
                                      if (atom.get_predicate().get_index() != constructor.get_data().predicate)
                                          return;

                                      const auto object = detail::object_index(atom, 0);
                                      assert(ygg::uint_t(object) < num_objects);
                                      bitset.set(ygg::uint_t(object));
                                  });

    return result;
}

template<FamilyTag Family, tyr::formalism::FactKind T, tyr::TaskKind Kind, typename C>
auto evaluate_atomic_state_role(ygg::View<ygg::Index<FamilyRole<Family, AtomicStateTag<T>>>, C> constructor, EvaluationContext<Family, Kind>& context)
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
                                         assert(ygg::uint_t(lhs) < num_objects);
                                         assert(ygg::uint_t(rhs) < num_objects);
                                         result->get(lhs).set(ygg::uint_t(rhs));
                                     });

    if (!constructor.get_polarity())
        for (ygg::uint_t object = 0; object < num_objects; ++object)
            result->get(object).flip();

    return result;
}

template<FamilyTag Family, tyr::formalism::FactKind T, tyr::TaskKind Kind, typename C>
auto evaluate_atomic_goal_role(ygg::View<ygg::Index<FamilyRole<Family, AtomicGoalTag<T>>>, C> constructor, EvaluationContext<Family, Kind>& context)
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
                                      assert(ygg::uint_t(lhs) < num_objects);
                                      assert(ygg::uint_t(rhs) < num_objects);
                                      result->get(lhs).set(ygg::uint_t(rhs));
                                  });

    return result;
}

template<FamilyTag Family, tyr::formalism::FactKind T, tyr::TaskKind Kind, typename C>
auto evaluate_atomic_state_boolean(ygg::View<ygg::Index<FamilyBoolean<Family, AtomicStateTag<T>>>, C> constructor, EvaluationContext<Family, Kind>& context)
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

template<FamilyTag Family, tyr::formalism::FactKind T, tyr::TaskKind Kind, typename C>
auto evaluate_atomic_goal_boolean(ygg::View<ygg::Index<FamilyBoolean<Family, AtomicGoalTag<T>>>, C> constructor, EvaluationContext<Family, Kind>& context)
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

template<FamilyTag Family, typename C, typename Evaluate>
bool evaluate_nonempty(ygg::View<ygg::Index<FamilyConstructor<Family, ConceptTag>>, C> constructor, Evaluate&& evaluate_child)
{
    return detail::deref(std::forward<Evaluate>(evaluate_child)(constructor)).get().any();
}

template<FamilyTag Family, typename C, typename Evaluate>
bool evaluate_nonempty(ygg::View<ygg::Index<FamilyConstructor<Family, RoleTag>>, C> constructor, Evaluate&& evaluate_child)
{
    return detail::deref(std::forward<Evaluate>(evaluate_child)(constructor)).any();
}

template<FamilyTag Family, typename C, typename Evaluate>
auto evaluate_count(ygg::View<ygg::Index<FamilyConstructor<Family, ConceptTag>>, C> constructor, Evaluate&& evaluate_child) -> ygg::uint_t
{
    return static_cast<ygg::uint_t>(detail::deref(std::forward<Evaluate>(evaluate_child)(constructor)).get().count());
}

template<FamilyTag Family, typename C, typename Evaluate>
auto evaluate_count(ygg::View<ygg::Index<FamilyConstructor<Family, RoleTag>>, C> constructor, Evaluate&& evaluate_child) -> ygg::uint_t
{
    return static_cast<ygg::uint_t>(detail::deref(std::forward<Evaluate>(evaluate_child)(constructor)).count());
}

template<ComparisonTag Tag>
constexpr bool apply_comparison(ygg::uint_t lhs, ygg::uint_t rhs) noexcept
{
    if constexpr (std::same_as<Tag, EqTag<comparison_operand_t<Tag>>>)
        return lhs == rhs;
    else if constexpr (std::same_as<Tag, NeqTag<comparison_operand_t<Tag>>>)
        return lhs != rhs;
    else if constexpr (std::same_as<Tag, LtTag<comparison_operand_t<Tag>>>)
        return lhs < rhs;
    else if constexpr (std::same_as<Tag, LeTag<comparison_operand_t<Tag>>>)
        return lhs <= rhs;
    else if constexpr (std::same_as<Tag, GtTag<comparison_operand_t<Tag>>>)
        return lhs > rhs;
    else if constexpr (std::same_as<Tag, GeTag<comparison_operand_t<Tag>>>)
        return lhs >= rhs;
}

template<LogicalBinaryTag Tag>
constexpr bool apply_logical_binary(bool lhs, bool rhs) noexcept
{
    if constexpr (std::same_as<Tag, AndTag>)
        return lhs && rhs;
    else if constexpr (std::same_as<Tag, OrTag>)
        return lhs || rhs;
}

// Numerical operands are nonnegative integers; numeric_limits<ygg::uint_t>::max() denotes infinity
// (as produced by the distance constructor for unreachable targets).
template<NumericalBinaryTag Tag>
constexpr ygg::uint_t apply_numerical_binary(ygg::uint_t lhs, ygg::uint_t rhs) noexcept
{
    constexpr auto inf = std::numeric_limits<ygg::uint_t>::max();
    if constexpr (std::same_as<Tag, AddTag>)
    {
        if (lhs == inf || rhs == inf)
            return inf;
        const ygg::uint_t result = lhs + rhs;
        return result < lhs ? inf : result;  // clamp overflow to infinity
    }
    else if constexpr (std::same_as<Tag, SubTag>)
    {
        if (lhs == inf)
            return inf;
        return lhs >= rhs ? lhs - rhs : 0;  // saturate at 0
    }
    else if constexpr (std::same_as<Tag, MulTag>)
    {
        if (lhs == 0 || rhs == 0)
            return 0;
        if (lhs == inf || rhs == inf)
            return inf;
        const ygg::uint_t result = lhs * rhs;
        return result / lhs != rhs ? inf : result;  // clamp overflow to infinity
    }
    else if constexpr (std::same_as<Tag, DivTag>)
    {
        if (rhs == 0)
            return inf;  // division by zero -> infinity
        if (lhs == inf)
            return inf;
        return lhs / rhs;
    }
    else if constexpr (std::same_as<Tag, MinTag>)
    {
        return lhs < rhs ? lhs : rhs;
    }
    else if constexpr (std::same_as<Tag, MaxTag>)
    {
        return lhs > rhs ? lhs : rhs;
    }
}

}  // namespace detail

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C, typename Evaluate>
    requires FamilyConceptConstructorTag<Family, Tag>
auto evaluate_concept(ygg::View<ygg::Index<FamilyConcept<Family, Tag>>, C> constructor,
                      EvaluationContext<Family, Kind>& context,
                      Evaluate&& evaluate_child) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<ConceptTag>>>
{
    [[maybe_unused]] const auto num_objects = detail::num_objects(context);
    auto result = detail::make_concept_builder(context);
    auto result_bitset = result->get();

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
        const auto lhs = evaluate_child(constructor.get_lhs());
        const auto rhs = evaluate_child(constructor.get_rhs());
        result_bitset.copy_from(detail::deref(lhs).get());
        result_bitset &= detail::deref(rhs).get();
    }
    else if constexpr (std::same_as<Tag, UnionTag>)
    {
        const auto lhs = evaluate_child(constructor.get_lhs());
        const auto rhs = evaluate_child(constructor.get_rhs());
        result_bitset.copy_from(detail::deref(lhs).get());
        result_bitset |= detail::deref(rhs).get();
    }
    else if constexpr (std::same_as<Tag, NegationTag>)
    {
        const auto arg = evaluate_child(constructor.get_arg());
        result_bitset.copy_from(detail::deref(arg).get());
        result_bitset.flip();
    }
    else if constexpr (std::same_as<Tag, ValueRestrictionTag>)
    {
        const auto role = evaluate_child(constructor.get_lhs());
        const auto concept_denotation = evaluate_child(constructor.get_rhs());
        const auto concept_bitset = detail::deref(concept_denotation).get();

        result_bitset.set();
        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            if (!detail::deref(role).get(object).is_subset_of(concept_bitset))
                result_bitset.reset(object);
        }
    }
    else if constexpr (std::same_as<Tag, ExistentialQuantificationTag>)
    {
        const auto role = evaluate_child(constructor.get_lhs());
        const auto concept_denotation = evaluate_child(constructor.get_rhs());
        const auto concept_bitset = detail::deref(concept_denotation).get();

        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            if (detail::deref(role).get(object).intersects(concept_bitset))
                result_bitset.set(object);
        }
    }
    else if constexpr (std::same_as<Tag, AtLeastNumberRestrictionTag> || std::same_as<Tag, AtMostNumberRestrictionTag>
                       || std::same_as<Tag, ExactNumberRestrictionTag>)
    {
        const auto role = evaluate_child(constructor.get_role());
        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            const auto count = static_cast<ygg::uint_t>(detail::deref(role).get(object).count());
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
        const auto role = evaluate_child(constructor.get_role());
        const auto concept_denotation = evaluate_child(constructor.get_concept());
        const auto concept_bitset = detail::deref(concept_denotation).get();

        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            auto count = ygg::uint_t { 0 };
            auto row = detail::deref(role).get(object);
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
        const auto lhs = evaluate_child(constructor.get_lhs());
        const auto rhs = evaluate_child(constructor.get_rhs());

        result_bitset.set();
        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            if (!detail::deref(lhs).get(object).is_subset_of(detail::deref(rhs).get(object)))
                result_bitset.reset(object);
        }
    }
    else if constexpr (std::same_as<Tag, AgreementTag>)
    {
        const auto lhs = evaluate_child(constructor.get_lhs());
        const auto rhs = evaluate_child(constructor.get_rhs());

        result_bitset.set();
        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            if (detail::deref(lhs).get(object) != detail::deref(rhs).get(object))
                result_bitset.reset(object);
        }
    }
    else if constexpr (std::same_as<Tag, NominalTag>)
    {
        const auto object = constructor.get_object().get_index();
        assert(ygg::uint_t(object) < num_objects);
        result_bitset.set(ygg::uint_t(object));
    }
    else if constexpr (std::same_as<Tag, RoleFillersTag>)
    {
        const auto role = evaluate_child(constructor.get_role());
        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            auto row = detail::deref(role).get(object);
            auto contains_all_fillers = true;
            for (auto filler : constructor.get_objects())
            {
                if (!row.test(ygg::uint_t(filler.get_index())))
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
            result_bitset.set(ygg::uint_t(object.get_index()));
    }
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL concept constructor tag in evaluate_impl");
    }

    return result;
}

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyConceptConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConcept<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<ConceptTag>>>
{
    return evaluate_concept(constructor, context, [&](auto child) { return evaluate_impl(child, context, workspace); });
}

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyConceptConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConcept<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace,
                   DenotationCaches<Family>& caches) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<ConceptTag>>>
{
    return evaluate_concept(constructor, context, [&](auto child) { return evaluate(child, context, workspace, caches); });
}

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C, typename Evaluate>
    requires FamilyRoleConstructorTag<Family, Tag>
auto evaluate_role(ygg::View<ygg::Index<FamilyRole<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   Evaluate&& evaluate_child) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<RoleTag>>>
{
    [[maybe_unused]] const auto num_objects = detail::num_objects(context);
    auto result = detail::make_role_builder(context);

    if constexpr (std::same_as<Tag, UniversalTag>)
    {
        for (ygg::uint_t object = 0; object < num_objects; ++object)
            result->get(object).set();
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
        const auto lhs = evaluate_child(constructor.get_lhs());
        const auto rhs = evaluate_child(constructor.get_rhs());

        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            auto row = result->get(object);
            row.copy_from(detail::deref(lhs).get(object));
            row &= detail::deref(rhs).get(object);
        }
    }
    else if constexpr (std::same_as<Tag, UnionTag>)
    {
        const auto lhs = evaluate_child(constructor.get_lhs());
        const auto rhs = evaluate_child(constructor.get_rhs());

        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            auto row = result->get(object);
            row.copy_from(detail::deref(lhs).get(object));
            row |= detail::deref(rhs).get(object);
        }
    }
    else if constexpr (std::same_as<Tag, ComplementTag>)
    {
        const auto arg = evaluate_child(constructor.get_arg());

        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            auto row = result->get(object);
            row.copy_from(detail::deref(arg).get(object));
            row.flip();
        }
    }
    else if constexpr (std::same_as<Tag, InverseTag>)
    {
        const auto arg = evaluate_child(constructor.get_arg());

        for (ygg::uint_t lhs = 0; lhs < num_objects; ++lhs)
        {
            const auto row = detail::deref(arg).get(lhs);
            for (auto rhs = row.find_first(); rhs != decltype(row)::npos; rhs = row.find_next(rhs))
                result->get(static_cast<ygg::uint_t>(rhs)).set(lhs);
        }
    }
    else if constexpr (std::same_as<Tag, CompositionTag>)
    {
        const auto lhs = evaluate_child(constructor.get_lhs());
        const auto rhs = evaluate_child(constructor.get_rhs());

        for (ygg::uint_t source = 0; source < num_objects; ++source)
        {
            auto result_row = result->get(source);
            const auto lhs_row = detail::deref(lhs).get(source);

            for (auto mid = lhs_row.find_first(); mid != decltype(lhs_row)::npos; mid = lhs_row.find_next(mid))
                result_row |= detail::deref(rhs).get(static_cast<ygg::uint_t>(mid));
        }
    }
    else if constexpr (std::same_as<Tag, TransitiveClosureTag> || std::same_as<Tag, ReflexiveTransitiveClosureTag>)
    {
        const auto arg = evaluate_child(constructor.get_arg());

        for (ygg::uint_t object = 0; object < num_objects; ++object)
            result->get(object).copy_from(detail::deref(arg).get(object));

        for (ygg::uint_t mid = 0; mid < num_objects; ++mid)
        {
            for (ygg::uint_t source = 0; source < num_objects; ++source)
            {
                auto source_row = result->get(source);
                if (source_row[mid])
                    source_row |= result->get(mid);
            }
        }

        if constexpr (std::same_as<Tag, ReflexiveTransitiveClosureTag>)
            for (ygg::uint_t object = 0; object < num_objects; ++object)
                result->get(object).set(object);
    }
    else if constexpr (std::same_as<Tag, RestrictionTag>)
    {
        const auto role = evaluate_child(constructor.get_lhs());
        const auto concept_denotation = evaluate_child(constructor.get_rhs());
        const auto concept_bitset = detail::deref(concept_denotation).get();

        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            auto row = result->get(object);
            row.copy_from(detail::deref(role).get(object));
            row &= concept_bitset;
        }
    }
    else if constexpr (std::same_as<Tag, IdentityTag>)
    {
        const auto concept_denotation = evaluate_child(constructor.get_arg());
        const auto bitset = detail::deref(concept_denotation).get();

        for (auto object = bitset.find_first(); object != decltype(bitset)::npos; object = bitset.find_next(object))
            result->get(static_cast<ygg::uint_t>(object)).set(object);
    }
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL role constructor tag in evaluate_impl");
    }

    return result;
}

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyRoleConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyRole<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<RoleTag>>>
{
    return evaluate_role(constructor, context, [&](auto child) { return evaluate_impl(child, context, workspace); });
}

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyRoleConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyRole<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace,
                   DenotationCaches<Family>& caches) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<RoleTag>>>
{
    return evaluate_role(constructor, context, [&](auto child) { return evaluate(child, context, workspace, caches); });
}

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C, typename Evaluate>
    requires FamilyBooleanConstructorTag<Family, Tag>
auto evaluate_boolean(ygg::View<ygg::Index<FamilyBoolean<Family, Tag>>, C> constructor,
                      EvaluationContext<Family, Kind>& context,
                      Evaluate&& evaluate_child) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<BooleanTag>>>
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
        const auto result_value = ygg::visit([&](auto arg) { return detail::evaluate_nonempty(arg, evaluate_child); }, constructor.get_arg());
        return context.get_builder().template get_builder<Denotation<BooleanTag>>(result_value);
    }
    else if constexpr (ComparisonTag<Tag>)
    {
        const auto lhs = evaluate_child(constructor.get_lhs());
        const auto rhs = evaluate_child(constructor.get_rhs());
        const auto lhs_value = static_cast<ygg::uint_t>(detail::deref(lhs).get());
        const auto rhs_value = static_cast<ygg::uint_t>(detail::deref(rhs).get());
        const bool result_value = detail::apply_comparison<Tag>(lhs_value, rhs_value);
        return context.get_builder().template get_builder<Denotation<BooleanTag>>(result_value);
    }
    else if constexpr (std::same_as<Tag, BooleanConstantTag>)
    {
        return context.get_builder().template get_builder<Denotation<BooleanTag>>(constructor.get_value());
    }
    else if constexpr (LogicalBinaryTag<Tag>)
    {
        const auto lhs = evaluate_child(constructor.get_lhs());
        const auto rhs = evaluate_child(constructor.get_rhs());
        const bool result_value = detail::apply_logical_binary<Tag>(detail::deref(lhs).get(), detail::deref(rhs).get());
        return context.get_builder().template get_builder<Denotation<BooleanTag>>(result_value);
    }
    else if constexpr (std::same_as<Tag, NotTag>)
    {
        const auto arg = evaluate_child(constructor.get_arg());
        return context.get_builder().template get_builder<Denotation<BooleanTag>>(!detail::deref(arg).get());
    }
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL boolean constructor tag in evaluate_impl");
    }
}

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyBooleanConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyBoolean<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<BooleanTag>>>
{
    return evaluate_boolean(constructor, context, [&](auto child) { return evaluate_impl(child, context, workspace); });
}

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyBooleanConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyBoolean<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace,
                   DenotationCaches<Family>& caches) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<BooleanTag>>>
{
    return evaluate_boolean(constructor, context, [&](auto child) { return evaluate(child, context, workspace, caches); });
}

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C, typename Evaluate>
    requires FamilyNumericalConstructorTag<Family, Tag>
auto evaluate_numerical(ygg::View<ygg::Index<FamilyNumerical<Family, Tag>>, C> constructor,
                        EvaluationContext<Family, Kind>& context,
                        EvaluationWorkspace& workspace,
                        Evaluate&& evaluate_child) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<NumericalTag>>>
{
    ygg::uint_t result_value = 0;

    if constexpr (std::same_as<Tag, CountTag>)
    {
        result_value = ygg::visit([&](auto arg) { return detail::evaluate_count(arg, evaluate_child); }, constructor.get_arg());
    }
    else if constexpr (std::same_as<Tag, DistanceTag>)
    {
        constexpr auto infinity = std::numeric_limits<ygg::uint_t>::max();

        const auto lhs = evaluate_child(constructor.get_lhs());
        const auto lhs_bitset = detail::deref(lhs).get();
        result_value = infinity;

        if (lhs_bitset.any())
        {
            const auto rhs = evaluate_child(constructor.get_rhs());
            const auto rhs_bitset = detail::deref(rhs).get();

            if (rhs_bitset.any())
            {
                if (lhs_bitset.intersects(rhs_bitset))
                {
                    result_value = 0;
                }
                else
                {
                    const auto role = evaluate_child(constructor.get_mid());
                    workspace.prepare_distance(static_cast<ygg::uint_t>(lhs_bitset.size()));
                    auto& queue = workspace.get_distance_queue();
                    auto& distances = workspace.get_distance_values();
                    size_t queue_pos = 0;

                    for (auto object = lhs_bitset.find_first(); object != decltype(lhs_bitset)::npos; object = lhs_bitset.find_next(object))
                    {
                        queue.push_back(static_cast<ygg::uint_t>(object));
                        distances[object] = 0;
                    }

                    while (queue_pos < queue.size())
                    {
                        const auto source = queue[queue_pos++];
                        const auto source_distance = distances[source];
                        assert(source_distance != infinity);

                        const auto row = detail::deref(role).get(source);
                        for (auto target = row.find_first(); target != decltype(row)::npos; target = row.find_next(target))
                        {
                            auto& target_distance = distances[target];
                            if (target_distance != infinity)
                                continue;

                            target_distance = source_distance + 1;
                            if (rhs_bitset[target])
                                return context.get_builder().template get_builder<Denotation<NumericalTag>>(target_distance);

                            queue.push_back(static_cast<ygg::uint_t>(target));
                        }
                    }
                }
            }
        }
    }
    else if constexpr (std::same_as<Tag, NumericalConstantTag>)
    {
        result_value = constructor.get_value();
    }
    else if constexpr (NumericalBinaryTag<Tag>)
    {
        const auto lhs = evaluate_child(constructor.get_lhs());
        const auto rhs = evaluate_child(constructor.get_rhs());
        result_value = detail::apply_numerical_binary<Tag>(detail::deref(lhs).get(), detail::deref(rhs).get());
    }
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL numerical constructor tag in evaluate_impl");
    }

    auto result = context.get_builder().template get_builder<Denotation<NumericalTag>>(result_value);
    return result;
}

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyNumericalConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyNumerical<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<NumericalTag>>>
{
    return evaluate_numerical(constructor, context, workspace, [&](auto child) { return evaluate_impl(child, context, workspace); });
}

template<FamilyTag Family, typename Tag, tyr::TaskKind Kind, typename C>
    requires FamilyNumericalConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyNumerical<Family, Tag>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace,
                   DenotationCaches<Family>& caches) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<NumericalTag>>>
{
    return evaluate_numerical(constructor, context, workspace, [&](auto child) { return evaluate(child, context, workspace, caches); });
}

template<FamilyTag Family, CategoryTag Category, tyr::TaskKind Kind>
auto evaluate_impl(FamilyConstructorView<Family, Category> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace,
                   DenotationCaches<Family>& caches) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<Category>>>
{
    return ygg::visit([&](auto child) { return evaluate_impl(child, context, workspace, caches); }, constructor.get_variant());
}

template<FamilyTag Family, CategoryTag Category, tyr::TaskKind Kind, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConstructor<Family, Category>>, C> constructor,
                   EvaluationContext<Family, Kind>& context,
                   EvaluationWorkspace& workspace) -> ygg::UniqueObjectPoolPtr<ygg::Builder<Denotation<Category>>>
{
    return ygg::visit([&](auto child) { return evaluate_impl(child, context, workspace); }, constructor.get_variant());
}

template<FamilyTag Family, CategoryTag Category, tyr::TaskKind Kind>
auto evaluate(FamilyConstructorView<Family, Category> constructor,
              EvaluationContext<Family, Kind>& context,
              EvaluationWorkspace& workspace,
              DenotationCaches<Family>& caches) -> DenotationView<Category>
{
    return with_cache<Category>(constructor,
                                caches,
                                [&]()
                                {
                                    auto result = evaluate_impl(constructor, context, workspace, caches);
                                    return detail::materialize_denotation<Category>(result, context).first;
                                });
}

template<FamilyTag Family, CategoryTag Category, tyr::TaskKind Kind>
auto evaluate(FamilyConstructorView<Family, Category> constructor,
              EvaluationContext<Family, Kind>& context,
              DenotationCaches<Family>& caches) -> DenotationView<Category>
{
    auto workspace = EvaluationWorkspace {};
    return evaluate(constructor, context, workspace, caches);
}

template<FamilyTag Family, CategoryTag Category, tyr::TaskKind Kind, typename C>
auto evaluate(ygg::View<ygg::Index<FamilyConstructor<Family, Category>>, C> constructor,
              EvaluationContext<Family, Kind>& context,
              EvaluationWorkspace& workspace)
{
    auto result = evaluate_impl(constructor, context, workspace);
    return detail::materialize_denotation<Category>(result, context).first;
}

template<FamilyTag Family, CategoryTag Category, tyr::TaskKind Kind, typename C>
auto evaluate(ygg::View<ygg::Index<FamilyConstructor<Family, Category>>, C> constructor, EvaluationContext<Family, Kind>& context)
{
    auto workspace = EvaluationWorkspace {};
    return evaluate(constructor, context, workspace);
}

}

#endif
