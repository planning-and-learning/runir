#ifndef RUNIR_KR_DL_SEMANTICS_DETAIL_EVALUATION_IMPL_HPP_
#define RUNIR_KR_DL_SEMANTICS_DETAIL_EVALUATION_IMPL_HPP_

#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/distance_evaluation.hpp"
#include "runir/kr/dl/semantics/evaluation_decl.hpp"
#include "runir/kr/dl/semantics/evaluation_storage.hpp"
#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/query_evaluation.hpp"

namespace runir::kr::dl::semantics
{

template<tyr::TaskKind Kind, FamilyTag Family, typename Tag, StateEvaluationContextConcept<Family, Kind> Context, typename C>
    requires FamilyConceptConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConcept<Family, Tag>>, C> constructor, Context& context) -> DenotationView<ConceptTag>
{
    [[maybe_unused]] auto children = context.child_context();
    auto& repository = context.get_denotation_repository();
    [[maybe_unused]] const auto num_objects = detail::num_objects<Family, Kind>(context);
    auto result = detail::make_concept_builder<Kind, Family>(context);
    auto result_bitset = result->get();

    if constexpr (std::same_as<Tag, BotTag>) {}
    else if constexpr (std::same_as<Tag, TopTag>)
    {
        result_bitset.set();
    }
    else if constexpr (is_atomic_state_tag_v<Tag>)
    {
        detail::evaluate_atomic_state_concept<Kind>(constructor, context, *result);
    }
    else if constexpr (is_atomic_goal_tag_v<Tag>)
    {
        detail::evaluate_atomic_goal_concept<Kind>(constructor, context, *result);
    }
    else if constexpr (std::same_as<Tag, IntersectionTag>)
    {
        const auto lhs = evaluate<Kind>(constructor.get_lhs(), children);
        const auto rhs = evaluate<Kind>(constructor.get_rhs(), children);
        result_bitset.copy_from(lhs.get());
        result_bitset &= rhs.get();
    }
    else if constexpr (std::same_as<Tag, UnionTag>)
    {
        const auto lhs = evaluate<Kind>(constructor.get_lhs(), children);
        const auto rhs = evaluate<Kind>(constructor.get_rhs(), children);
        result_bitset.copy_from(lhs.get());
        result_bitset |= rhs.get();
    }
    else if constexpr (std::same_as<Tag, NegationTag>)
    {
        const auto arg = evaluate<Kind>(constructor.get_arg(), children);
        result_bitset.copy_from(arg.get());
        result_bitset.flip();
    }
    else if constexpr (std::same_as<Tag, ValueRestrictionTag>)
    {
        const auto role = evaluate<Kind>(constructor.get_lhs(), children);
        const auto concept_denotation = evaluate<Kind>(constructor.get_rhs(), children);
        const auto concept_bitset = concept_denotation.get();

        result_bitset.set();
        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            if (!role.get(object).is_subset_of(concept_bitset))
                result_bitset.reset(object);
        }
    }
    else if constexpr (std::same_as<Tag, ExistentialQuantificationTag>)
    {
        const auto role = evaluate<Kind>(constructor.get_lhs(), children);
        const auto concept_denotation = evaluate<Kind>(constructor.get_rhs(), children);
        const auto concept_bitset = concept_denotation.get();

        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            if (role.get(object).intersects(concept_bitset))
                result_bitset.set(object);
        }
    }
    else if constexpr (std::same_as<Tag, AtLeastNumberRestrictionTag> || std::same_as<Tag, AtMostNumberRestrictionTag>
                       || std::same_as<Tag, ExactNumberRestrictionTag>)
    {
        const auto role = evaluate<Kind>(constructor.get_role(), children);
        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            const auto count = static_cast<ygg::uint_t>(role.get(object).count());
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
        const auto role = evaluate<Kind>(constructor.get_role(), children);
        const auto concept_denotation = evaluate<Kind>(constructor.get_concept(), children);
        const auto concept_bitset = concept_denotation.get();

        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            const auto count = role.get(object).count_intersection(concept_bitset);
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
        const auto lhs = evaluate<Kind>(constructor.get_lhs(), children);
        const auto rhs = evaluate<Kind>(constructor.get_rhs(), children);

        result_bitset.set();
        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            if (!lhs.get(object).is_subset_of(rhs.get(object)))
                result_bitset.reset(object);
        }
    }
    else if constexpr (std::same_as<Tag, AgreementTag>)
    {
        const auto lhs = evaluate<Kind>(constructor.get_lhs(), children);
        const auto rhs = evaluate<Kind>(constructor.get_rhs(), children);

        result_bitset.set();
        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            if (lhs.get(object) != rhs.get(object))
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
        const auto role = evaluate<Kind>(constructor.get_role(), children);
        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            auto row = role.get(object);
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

    return insert(repository, *result, context.get_builder()).first;
}

template<tyr::TaskKind Kind, FamilyTag Family, typename Tag, StateEvaluationContextConcept<Family, Kind> Context, typename C>
    requires FamilyRoleConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyRole<Family, Tag>>, C> constructor, Context& context) -> DenotationView<RoleTag>
{
    [[maybe_unused]] auto children = context.child_context();
    auto& repository = context.get_denotation_repository();
    [[maybe_unused]] const auto num_objects = detail::num_objects<Family, Kind>(context);
    auto result = detail::make_role_builder<Kind, Family>(context);

    if constexpr (std::same_as<Tag, UniversalTag>)
    {
        for (ygg::uint_t object = 0; object < num_objects; ++object)
            result->get(object).set();
    }
    else if constexpr (is_atomic_state_tag_v<Tag>)
    {
        detail::evaluate_atomic_state_role<Kind>(constructor, context, *result);
    }
    else if constexpr (is_atomic_goal_tag_v<Tag>)
    {
        detail::evaluate_atomic_goal_role<Kind>(constructor, context, *result);
    }
    else if constexpr (std::same_as<Tag, IntersectionTag>)
    {
        const auto lhs = evaluate<Kind>(constructor.get_lhs(), children);
        const auto rhs = evaluate<Kind>(constructor.get_rhs(), children);

        auto bits = result->storage_bits();
        bits.copy_from(lhs.storage_bits());
        bits &= rhs.storage_bits();
    }
    else if constexpr (std::same_as<Tag, UnionTag>)
    {
        const auto lhs = evaluate<Kind>(constructor.get_lhs(), children);
        const auto rhs = evaluate<Kind>(constructor.get_rhs(), children);

        auto bits = result->storage_bits();
        bits.copy_from(lhs.storage_bits());
        bits |= rhs.storage_bits();
    }
    else if constexpr (std::same_as<Tag, ComplementTag>)
    {
        const auto arg = evaluate<Kind>(constructor.get_arg(), children);

        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            auto row = result->get(object);
            row.copy_from(arg.get(object));
            row.flip();
        }
    }
    else if constexpr (std::same_as<Tag, InverseTag>)
    {
        const auto arg = evaluate<Kind>(constructor.get_arg(), children);

        for (ygg::uint_t lhs = 0; lhs < num_objects; ++lhs)
        {
            const auto row = arg.get(lhs);
            for (const auto rhs : ygg::set_bit_indices(row))
                result->get(static_cast<ygg::uint_t>(rhs)).set(lhs);
        }
    }
    else if constexpr (std::same_as<Tag, CompositionTag>)
    {
        const auto lhs = evaluate<Kind>(constructor.get_lhs(), children);
        const auto rhs = evaluate<Kind>(constructor.get_rhs(), children);

        for (ygg::uint_t source = 0; source < num_objects; ++source)
        {
            auto result_row = result->get(source);
            const auto lhs_row = lhs.get(source);

            for (const auto mid : ygg::set_bit_indices(lhs_row))
                result_row |= rhs.get(static_cast<ygg::uint_t>(mid));
        }
    }
    else if constexpr (std::same_as<Tag, TransitiveClosureTag> || std::same_as<Tag, ReflexiveTransitiveClosureTag>)
    {
        const auto arg = evaluate<Kind>(constructor.get_arg(), children);

        assign(*result, arg);

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
        const auto role = evaluate<Kind>(constructor.get_lhs(), children);
        const auto concept_denotation = evaluate<Kind>(constructor.get_rhs(), children);
        const auto concept_bitset = concept_denotation.get();

        for (ygg::uint_t object = 0; object < num_objects; ++object)
        {
            auto row = result->get(object);
            row.copy_from(role.get(object));
            row &= concept_bitset;
        }
    }
    else if constexpr (std::same_as<Tag, IdentityTag>)
    {
        const auto concept_denotation = evaluate<Kind>(constructor.get_arg(), children);
        const auto bitset = concept_denotation.get();

        for (const auto object : ygg::set_bit_indices(bitset))
            result->get(static_cast<ygg::uint_t>(object)).set(object);
    }
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL role constructor tag in evaluate_impl");
    }

    return insert(repository, *result, context.get_builder()).first;
}

template<tyr::TaskKind Kind, FamilyTag Family, typename Tag, StateEvaluationContextConcept<Family, Kind> Context, typename C>
    requires FamilyBooleanConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyBoolean<Family, Tag>>, C> constructor, Context& context) -> DenotationView<BooleanTag>
{
    [[maybe_unused]] auto children = context.child_context();
    auto& repository = context.get_denotation_repository();
    bool result_value = false;
    if constexpr (is_atomic_state_tag_v<Tag>)
    {
        result_value = detail::evaluate_atomic_state_boolean<Kind>(constructor, context);
    }
    else if constexpr (is_atomic_goal_tag_v<Tag>)
    {
        result_value = detail::evaluate_atomic_goal_boolean<Kind>(constructor, context);
    }
    else if constexpr (std::same_as<Tag, NonemptyTag>)
    {
        result_value = ygg::visit([&](auto arg) { return detail::nonempty(evaluate<Kind>(arg, children)); }, constructor.get_arg());
    }
    else if constexpr (ComparisonTag<Tag>)
    {
        const auto lhs = evaluate<Kind>(constructor.get_lhs(), children);
        const auto rhs = evaluate<Kind>(constructor.get_rhs(), children);
        const auto lhs_value = static_cast<ygg::uint_t>(lhs.get());
        const auto rhs_value = static_cast<ygg::uint_t>(rhs.get());
        result_value = detail::apply_comparison<Tag>(lhs_value, rhs_value);
    }
    else if constexpr (std::same_as<Tag, BooleanConstantTag>)
    {
        result_value = constructor.get_value();
    }
    else if constexpr (LogicalBinaryTag<Tag>)
    {
        const auto lhs = evaluate<Kind>(constructor.get_lhs(), children);
        const auto rhs = evaluate<Kind>(constructor.get_rhs(), children);
        result_value = detail::apply_logical_binary<Tag>(lhs.get(), rhs.get());
    }
    else if constexpr (std::same_as<Tag, NotTag>)
    {
        const auto arg = evaluate<Kind>(constructor.get_arg(), children);
        result_value = !arg.get();
    }
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL boolean constructor tag in evaluate_impl");
    }

    auto result = context.get_builder().template get_builder<Denotation<BooleanTag>>(result_value);
    return insert(repository, *result, context.get_builder()).first;
}

template<tyr::TaskKind Kind, FamilyTag Family, typename Tag, StateEvaluationContextConcept<Family, Kind> Context, typename C>
    requires FamilyNumericalConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyNumerical<Family, Tag>>, C> constructor, Context& context) -> DenotationView<NumericalTag>
{
    [[maybe_unused]] auto children = context.child_context();
    auto& repository = context.get_denotation_repository();
    ygg::uint_t result_value = 0;

    if constexpr (std::same_as<Tag, CountTag>)
    {
        result_value = ygg::visit([&](auto arg) { return detail::count(evaluate<Kind>(arg, children)); }, constructor.get_arg());
    }
    else if constexpr (std::same_as<Tag, DistanceTag>)
    {
        result_value = detail::on_demand_distance<Kind>(constructor, context);
    }
    else if constexpr (std::same_as<Tag, NumericalConstantTag>)
    {
        result_value = constructor.get_value();
    }
    else if constexpr (NumericalBinaryTag<Tag>)
    {
        const auto lhs = evaluate<Kind>(constructor.get_lhs(), children);
        const auto rhs = evaluate<Kind>(constructor.get_rhs(), children);
        result_value = detail::apply_numerical_binary<Tag>(lhs.get(), rhs.get());
    }
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL numerical constructor tag in evaluate_impl");
    }

    auto result = context.get_builder().template get_builder<Denotation<NumericalTag>>(result_value);
    return insert(repository, *result, context.get_builder()).first;
}

template<tyr::TaskKind Kind, FamilyTag Family, CategoryTag Category, StateEvaluationContextConcept<Family, Kind> Context>
auto evaluate_impl(FamilyConstructorView<Family, Category> constructor, Context& context) -> DenotationView<Category>
{
    return ygg::visit([&](auto child) { return evaluate_impl<Kind>(child, context); }, constructor.get_variant());
}

template<tyr::TaskKind Kind, FamilyTag Family, CategoryTag Category, StateEvaluationContextConcept<Family, Kind> Context>
auto evaluate(FamilyConstructorView<Family, Category> constructor, Context& context) -> DenotationView<Category>
{
    const auto is_static = constructor.is_static();
    auto& cache = context.get_caches().template get<Category>(is_static);
    if (const auto it = cache.find(constructor); it != cache.end())
        return it->second;

    auto result_context = context.for_result(is_static);
    return cache.emplace(constructor, evaluate_impl<Kind>(constructor, result_context)).first->second;
}

template<tyr::TaskKind Kind, FamilyTag Family, typename C, StateEvaluationContextConcept<Family, Kind> Context>
auto evaluate_impl(ygg::View<ygg::Index<Query<Family, QueryRenameTag>>, C> constructor, Context& context) -> QueryDenotationView
{
    // Keep rows in their owning output partition, even when a low-level query
    // wrapper conservatively marks a static child as dynamic.
    const auto child = evaluate<Kind>(constructor.get_arg(), context);
    auto static_output = context.for_result(true);
    auto dynamic_output = context.for_result(false);
    auto& static_results = static_output.get_denotation_repository().get_relation_repository();
    auto& results = &child.get_context() == &static_results ? static_results : dynamic_output.get_denotation_repository().get_relation_repository();
    return results.rename(child, constructor.get_schema().span());
}

template<tyr::TaskKind Kind, FamilyTag Family, typename Tag, typename C, StateEvaluationContextConcept<Family, Kind> Context>
    requires(!std::same_as<Tag, void> && !std::same_as<Tag, QueryRenameTag>)
auto evaluate_impl(ygg::View<ygg::Index<Query<Family, Tag>>, C> constructor, Context& context) -> QueryDenotationView
{
    auto result = context.get_builder().template get_builder<ygg::database::Relation<QueryValues>>(constructor.get_schema().span());
    detail::evaluate_query<Kind>(constructor, context, *result);
    return insert(context.get_denotation_repository(), *result).first;
}

template<tyr::TaskKind Kind, FamilyTag Family, StateEvaluationContextConcept<Family, Kind> Context, typename C>
auto evaluate(ygg::View<ygg::Index<Query<Family>>, C> constructor, Context& context) -> QueryDenotationView
{
    auto& cache = context.get_caches().get_queries(constructor.is_static());
    if (const auto it = cache.find(constructor); it != cache.end())
        return it->second;

    auto output = context.for_result(constructor.is_static());
    const auto result = ygg::visit([&](auto child) { return evaluate_impl<Kind>(child, output); }, constructor.get_variant());
    return cache.emplace(constructor, result).first->second;
}

template<tyr::TaskKind Kind, FamilyTag Family, ConceptOrRoleTag Category, StateEvaluationContextConcept<Family, Kind> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<QueryProjection<Family, Category>>, C> constructor, Context& context) -> DenotationView<Category>
{
    const auto& columns = constructor.get_data().columns;
    auto children = context.child_context();
    const auto relation = evaluate<Kind>(constructor.get_arg(), children);
    const auto position = [&](size_t i) { return relation.column_index(ygg::Index<ygg::database::Column>(columns[i].get_value())); };
    auto result = context.get_builder().template get_builder<Denotation<Category>>(detail::num_objects<Family, Kind>(context));
    if constexpr (std::same_as<Category, ConceptTag>)
    {
        auto bits = result->get();
        const auto object = position(0);
        for (const auto row : relation)
            bits.set(row.template get<ygg::Index<tyr::formalism::Object>>(object).get_value());
    }
    else
    {
        static_assert(std::same_as<Category, RoleTag>);
        const auto source = position(0);
        const auto target = position(1);
        for (const auto row : relation)
            result->get(row.template get<ygg::Index<tyr::formalism::Object>>(source).get_value())
                .set(row.template get<ygg::Index<tyr::formalism::Object>>(target).get_value());
    }
    return insert(context.get_denotation_repository(), *result, context.get_builder()).first;
}

template<tyr::TaskKind Kind, StateEvaluationContextConcept<runir::kr::ExtFamilyTag, Kind> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConcept<runir::kr::ExtFamilyTag, RegisterTag>>, C> constructor, Context& context) -> DenotationView<ConceptTag>
{
    auto result = detail::make_concept_builder<Kind, runir::kr::ExtFamilyTag>(context);
    auto result_bitset = result->get();

    const auto object = context.registers().at(constructor.get_register().get_identifier());
    if (object)
        result_bitset.set(ygg::uint_t(object.value().get_index()));

    return insert(context.get_denotation_repository(), *result, context.get_builder()).first;
}

template<tyr::TaskKind Kind, StateEvaluationContextConcept<runir::kr::ExtFamilyTag, Kind> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyRole<runir::kr::ExtFamilyTag, RegisterTag>>, C> constructor, Context& context) -> DenotationView<RoleTag>
{
    auto result = detail::make_role_builder<Kind, runir::kr::ExtFamilyTag>(context);

    const auto value = context.registers().at(constructor.get_register().get_identifier());
    if (value)
    {
        const auto pair = value.value();
        result->get(pair.get_first().get_index()).set(ygg::uint_t(pair.get_second().get_index()));
    }

    return insert(context.get_denotation_repository(), *result, context.get_builder()).first;
}

template<tyr::TaskKind Kind, StateEvaluationContextConcept<runir::kr::ExtFamilyTag, Kind> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConcept<runir::kr::ExtFamilyTag, ArgumentTag<ConceptTag>>>, C> constructor,
                   Context& context) -> DenotationView<ConceptTag>
{
    return context.arguments().at(constructor.get_argument().get_identifier());
}

template<tyr::TaskKind Kind, StateEvaluationContextConcept<runir::kr::ExtFamilyTag, Kind> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyRole<runir::kr::ExtFamilyTag, ArgumentTag<RoleTag>>>, C> constructor, Context& context) -> DenotationView<RoleTag>
{
    return context.arguments().at(constructor.get_argument().get_identifier());
}

template<tyr::TaskKind Kind, StateEvaluationContextConcept<runir::kr::ExtFamilyTag, Kind> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyBoolean<runir::kr::ExtFamilyTag, ArgumentTag<BooleanTag>>>, C> constructor,
                   Context& context) -> DenotationView<BooleanTag>
{
    return context.arguments().at(constructor.get_argument().get_identifier());
}

template<tyr::TaskKind Kind, StateEvaluationContextConcept<runir::kr::ExtFamilyTag, Kind> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyNumerical<runir::kr::ExtFamilyTag, ArgumentTag<NumericalTag>>>, C> constructor,
                   Context& context) -> DenotationView<NumericalTag>
{
    return context.arguments().at(constructor.get_argument().get_identifier());
}

}  // namespace runir::kr::dl::semantics

#endif
