#ifndef RUNIR_KR_DL_SEMANTICS_DETAIL_EVALUATION_HELPERS_HPP_
#define RUNIR_KR_DL_SEMANTICS_DETAIL_EVALUATION_HELPERS_HPP_

/// Building blocks of the evaluator definitions, shared with the distance, query and incremental evaluators.

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotations.hpp"
#include "runir/kr/dl/semantics/evaluation_decl.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"
#include "runir/kr/dl/semantics/interning.hpp"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/planning/conjunctive_condition_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/ground/state_repository.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/state_repository.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <utility>
#include <yggdrasil/containers/dynamic_bitset.hpp>
#include <yggdrasil/containers/unique_object_pool.hpp>
#include <yggdrasil/core/byte_codec.hpp>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/database/operators/distance/distance.hpp>

namespace runir::kr::dl::semantics::detail
{

template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context>
auto num_objects(const Context& context) noexcept -> ygg::uint_t
{
    const auto task = context.get_state().get_task().get_task();
    return static_cast<ygg::uint_t>(task.get_num_objects());
}

template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context>
auto make_concept_builder(Context& context)
{ return context.get_builder().template get_builder<Denotation<ConceptTag>>(num_objects<Kind>(context)); }

template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context>
auto make_role_builder(Context& context)
{ return context.get_builder().template get_builder<Denotation<RoleTag>>(num_objects<Kind>(context)); }

template<tyr::formalism::FactKind T>
auto object_index(tyr::formalism::planning::AtomView<::tyr::GroundTag, T> atom, size_t position) -> ygg::Index<tyr::formalism::Object>
{ return atom.get_row().get_objects()[position].get_index(); }

template<tyr::TaskKind Kind, tyr::formalism::FactKind T, StateEvaluationContextConcept<Kind> Context, typename C>
void evaluate_atomic_state_concept(ygg::View<ygg::Index<Concept<AtomicStateTag<T>>>, C> constructor,
                                   Context& context,
                                   ygg::Builder<Denotation<ConceptTag>>& result)
{
    [[maybe_unused]] const auto num_objects = detail::num_objects<Kind>(context);
    auto bitset = result.get();

    const auto predicate = constructor.get_predicate();
    for (const auto atom : context.get_state().get_atoms_view(predicate))
    {
        const auto object = detail::object_index(atom, 0);
        assert(ygg::uint_t(object) < num_objects);
        bitset.set(ygg::uint_t(object));
    }

    if (!constructor.get_polarity())
        bitset.flip();
}

template<tyr::TaskKind Kind, tyr::formalism::FactKind T, StateEvaluationContextConcept<Kind> Context, typename C>
void evaluate_atomic_goal_concept(ygg::View<ygg::Index<Concept<AtomicGoalTag<T>>>, C> constructor,
                                  Context& context,
                                  ygg::Builder<Denotation<ConceptTag>>& result)
{
    [[maybe_unused]] const auto num_objects = detail::num_objects<Kind>(context);
    auto bitset = result.get();

    const auto predicate = constructor.get_predicate();
    for (const auto atom : context.get_state().get_task().get_task().get_goal().template get_atoms_view<T>(constructor.get_polarity(), predicate))
    {
        const auto object = detail::object_index(atom, 0);
        assert(ygg::uint_t(object) < num_objects);
        bitset.set(ygg::uint_t(object));
    }
}

template<tyr::TaskKind Kind, tyr::formalism::FactKind T, StateEvaluationContextConcept<Kind> Context, typename C>
void evaluate_atomic_state_role(ygg::View<ygg::Index<Role<AtomicStateTag<T>>>, C> constructor, Context& context, ygg::Builder<Denotation<RoleTag>>& result)
{
    [[maybe_unused]] const auto num_objects = detail::num_objects<Kind>(context);

    const auto predicate = constructor.get_predicate();
    for (const auto atom : context.get_state().get_atoms_view(predicate))
    {
        const auto lhs = detail::object_index(atom, 0);
        const auto rhs = detail::object_index(atom, 1);
        assert(ygg::uint_t(lhs) < num_objects);
        assert(ygg::uint_t(rhs) < num_objects);
        result.get(lhs).set(ygg::uint_t(rhs));
    }

    if (!constructor.get_polarity())
        for (ygg::uint_t object = 0; object < num_objects; ++object)
            result.get(object).flip();
}

template<tyr::TaskKind Kind, tyr::formalism::FactKind T, StateEvaluationContextConcept<Kind> Context, typename C>
void evaluate_atomic_goal_role(ygg::View<ygg::Index<Role<AtomicGoalTag<T>>>, C> constructor, Context& context, ygg::Builder<Denotation<RoleTag>>& result)
{
    [[maybe_unused]] const auto num_objects = detail::num_objects<Kind>(context);

    const auto predicate = constructor.get_predicate();
    for (const auto atom : context.get_state().get_task().get_task().get_goal().template get_atoms_view<T>(constructor.get_polarity(), predicate))
    {
        const auto lhs = detail::object_index(atom, 0);
        const auto rhs = detail::object_index(atom, 1);
        assert(ygg::uint_t(lhs) < num_objects);
        assert(ygg::uint_t(rhs) < num_objects);
        result.get(lhs).set(ygg::uint_t(rhs));
    }
}

template<tyr::TaskKind Kind, tyr::formalism::FactKind T, StateEvaluationContextConcept<Kind> Context, typename C>
bool evaluate_atomic_state_boolean(ygg::View<ygg::Index<Boolean<AtomicStateTag<T>>>, C> constructor, Context& context)
{
    auto atoms = context.get_state().get_atoms_view(constructor.get_predicate());
    const auto value = atoms.begin() != atoms.end();
    return constructor.get_polarity() ? value : !value;
}

template<tyr::TaskKind Kind, tyr::formalism::FactKind T, StateEvaluationContextConcept<Kind> Context, typename C>
bool evaluate_atomic_goal_boolean(ygg::View<ygg::Index<Boolean<AtomicGoalTag<T>>>, C> constructor, Context& context)
{
    auto atoms = context.get_state().get_task().get_task().get_goal().template get_atoms_view<T>(constructor.get_polarity(), constructor.get_predicate());
    return atoms.begin() != atoms.end();
}

inline bool nonempty(DenotationView<ConceptTag> value) { return value.get().any(); }
inline bool nonempty(DenotationView<RoleTag> value) { return value.any(); }
inline bool nonempty(QueryDenotationView value) { return !value.empty(); }

inline ygg::uint_t count(DenotationView<ConceptTag> value) { return ygg::to_uint_t(value.get().count()); }
inline ygg::uint_t count(DenotationView<RoleTag> value) { return ygg::to_uint_t(value.count()); }
inline ygg::uint_t count(QueryDenotationView value) { return ygg::to_uint_t(value.size()); }

/// The least value in the one column of a relation, the largest uint_t (infinity) if it has no row.
template<typename Rows>
ygg::uint_t least_value(const Rows& rows)
{
    auto result = std::numeric_limits<ygg::uint_t>::max();
    for (size_t i = 0; i < rows.size(); ++i)
        result = std::min(result, ygg::ByteCodec<ygg::uint_t>::decode(rows.row(i).first(ygg::ByteCodec<ygg::uint_t>::size)));
    return result;
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
        return std::min(lhs, rhs);
    }
    else if constexpr (std::same_as<Tag, MaxTag>)
    {
        return std::max(lhs, rhs);
    }
}

/// Inserts a concept's objects or a role's pairs as rows of a 1- or 2-column relation.
template<ConceptOrRoleTag Category, DenotationViewConcept<Category> Denotation>
void insert_denotation_rows(const Denotation& denotation, ygg::Builder<ygg::database::Relation>& result)
{
    for (const auto element : denotation.indices())
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            result.insert(std::tuple { element.get_value() });
        else
        {
            const auto [source, target] = element;
            result.insert(std::tuple { source.get_value(), target.get_value() });
        }
    }
}

/// A distance argument as a relation over the plan's columns for it.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context, typename C>
auto evaluate_relation(ygg::View<ygg::Index<Query<>>, C> argument, std::span<const ygg::database::ColumnLayout>, Context& context) -> QueryDenotationView
{ return evaluate<Kind>(argument, context); }

/// Concepts and roles are lifted, so all argument forms share one distance evaluation.
template<tyr::TaskKind Kind, ConceptOrRoleTag Category, StateEvaluationContextConcept<Kind> Context, typename C>
auto evaluate_relation(ygg::View<ygg::Index<Constructor<Category>>, C> argument, std::span<const ygg::database::ColumnLayout> columns, Context& context)
    -> QueryDenotationView
{
    const auto denotation = evaluate<Kind>(argument, context);
    auto result = context.get_builder().template get_builder<ygg::database::Relation>(columns);
    insert_denotation_rows<Category>(denotation, *result);
    return insert(context.get_denotation_repository(), *result).first;
}

}  // namespace runir::kr::dl::semantics::detail

#endif
