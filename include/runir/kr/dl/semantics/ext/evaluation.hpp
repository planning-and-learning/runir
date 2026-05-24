#ifndef RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_HPP_

#include "runir/kr/dl/concept_data.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/ext/evaluation_context.hpp"

namespace runir::kr::dl::semantics
{

namespace detail
{

template<CategoryTag Category, tyr::planning::TaskKind Kind>
auto copy_argument_denotation(EvaluationContext<ExtFamilyTag, Kind>& context,
                              tyr::Index<Denotation<Category>> denotation) -> EvaluationBuilderT<Category, ExtFamilyTag, Kind>
{
    const auto view = tyr::make_view(denotation, context.get_denotation_repository());

    if constexpr (std::same_as<Category, BooleanTag> || std::same_as<Category, NumericalTag>)
    {
        return context.get_builder().template get_builder<Denotation<Category>>(view.get());
    }
    else if constexpr (std::same_as<Category, ConceptTag>)
    {
        auto result = make_concept_builder(context);
        result->get_bitset().copy_from(view.get());
        return result;
    }
    else if constexpr (std::same_as<Category, RoleTag>)
    {
        auto result = make_role_builder(context);
        for (uint_t object = 0; object < result->num_objects; ++object)
            row(result, object).copy_from(view.get(tyr::Index<tyr::formalism::Object>(object)));
        return result;
    }
}

}  // namespace detail

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyConcept<ExtFamilyTag, RegisterTag>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> EvaluationBuilderT<ConceptTag, ExtFamilyTag, Kind>
{
    auto result = detail::make_concept_builder(context);
    auto result_bitset = result->get_bitset();

    const auto& object = context.at(constructor.get_identifier());
    if (object)
        result_bitset.set(tyr::uint_t(object->get_index()));

    return result;
}

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyRole<ExtFamilyTag, RegisterTag>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> EvaluationBuilderT<RoleTag, ExtFamilyTag, Kind>
{
    auto result = detail::make_role_builder(context);

    const auto& value = context.at(constructor.get_identifier());
    if (value)
    {
        const auto& [source, target] = *value;
        detail::row(result, source.get_index()).set(tyr::uint_t(target.get_index()));
    }

    return result;
}

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyConcept<ExtFamilyTag, ArgumentTag<ConceptTag>>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> EvaluationBuilderT<ConceptTag, ExtFamilyTag, Kind>
{
    return detail::copy_argument_denotation<ConceptTag>(context, context.argument(constructor.get_identifier()));
}

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyRole<ExtFamilyTag, ArgumentTag<RoleTag>>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> EvaluationBuilderT<RoleTag, ExtFamilyTag, Kind>
{
    return detail::copy_argument_denotation<RoleTag>(context, context.argument(constructor.get_identifier()));
}

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyBoolean<ExtFamilyTag, ArgumentTag<BooleanTag>>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> EvaluationBuilderT<BooleanTag, ExtFamilyTag, Kind>
{
    return detail::copy_argument_denotation<BooleanTag>(context, context.argument(constructor.get_identifier()));
}

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyNumerical<ExtFamilyTag, ArgumentTag<NumericalTag>>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> EvaluationBuilderT<NumericalTag, ExtFamilyTag, Kind>
{
    return detail::copy_argument_denotation<NumericalTag>(context, context.argument(constructor.get_identifier()));
}

}  // namespace runir::kr::dl::semantics

namespace runir::kr::dl::semantics::ext
{

template<tyr::planning::TaskKind Kind>
using EvaluationContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>;

template<tyr::planning::TaskKind Kind>
const auto& get_repository(const EvaluationContext<Kind>& context) noexcept
{
    return runir::kr::dl::semantics::get_repository(context);
}

using runir::kr::dl::semantics::evaluate;
using runir::kr::dl::semantics::evaluate_impl;

}  // namespace runir::kr::dl::semantics::ext

#endif
