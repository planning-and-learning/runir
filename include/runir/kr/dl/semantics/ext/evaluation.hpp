#ifndef RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_HPP_

#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/ext/evaluation_context.hpp"

#include <yggdrasil/core/dependent_false.hpp>

namespace runir::kr::dl::semantics
{

namespace detail
{

template<CategoryTag Category, tyr::planning::TaskKind Kind>
auto copy_argument_denotation(EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context,
                              const ygg::View<ygg::Index<Denotation<Category>>, DenotationRepository>& view)
    -> EvaluationBuilderT<Category, runir::kr::ExtFamilyTag, Kind>
{
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
            row(result, object).copy_from(view.get(ygg::Index<tyr::formalism::Object>(object)));
        return result;
    }
    else
    {
        static_assert(ygg::dependent_false<Category>::value, "unhandled DL denotation category in copy_argument_denotation");
    }
}

}  // namespace detail

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConcept<runir::kr::ExtFamilyTag, RegisterTag>>, C> constructor,
                   EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> EvaluationBuilderT<ConceptTag, runir::kr::ExtFamilyTag, Kind>
{
    auto result = detail::make_concept_builder(context);
    auto result_bitset = result->get_bitset();

    const auto& object = context.at(constructor.get_data().identifier);
    if (object)
        result_bitset.set(ygg::uint_t(object->get_index()));

    return result;
}

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyRole<runir::kr::ExtFamilyTag, RegisterTag>>, C> constructor,
                   EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> EvaluationBuilderT<RoleTag, runir::kr::ExtFamilyTag, Kind>
{
    auto result = detail::make_role_builder(context);

    const auto& value = context.at(constructor.get_data().identifier);
    if (value)
    {
        const auto& [source, target] = *value;
        detail::row(result, source.get_index()).set(ygg::uint_t(target.get_index()));
    }

    return result;
}

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConcept<runir::kr::ExtFamilyTag, ArgumentTag<ConceptTag>>>, C> constructor,
                   EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> EvaluationBuilderT<ConceptTag, runir::kr::ExtFamilyTag, Kind>
{
    return detail::copy_argument_denotation<ConceptTag>(context, context.argument(constructor.get_data().identifier));
}

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyRole<runir::kr::ExtFamilyTag, ArgumentTag<RoleTag>>>, C> constructor,
                   EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> EvaluationBuilderT<RoleTag, runir::kr::ExtFamilyTag, Kind>
{
    return detail::copy_argument_denotation<RoleTag>(context, context.argument(constructor.get_data().identifier));
}

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyBoolean<runir::kr::ExtFamilyTag, ArgumentTag<BooleanTag>>>, C> constructor,
                   EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> EvaluationBuilderT<BooleanTag, runir::kr::ExtFamilyTag, Kind>
{
    return detail::copy_argument_denotation<BooleanTag>(context, context.argument(constructor.get_data().identifier));
}

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyNumerical<runir::kr::ExtFamilyTag, ArgumentTag<NumericalTag>>>, C> constructor,
                   EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> EvaluationBuilderT<NumericalTag, runir::kr::ExtFamilyTag, Kind>
{
    return detail::copy_argument_denotation<NumericalTag>(context, context.argument(constructor.get_data().identifier));
}

}  // namespace runir::kr::dl::semantics

namespace runir::kr::dl::semantics::ext
{

template<tyr::planning::TaskKind Kind>
const auto& get_repository(const runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context) noexcept
{
    return runir::kr::dl::semantics::get_repository(context);
}

using runir::kr::dl::semantics::evaluate;
using runir::kr::dl::semantics::evaluate_impl;

}  // namespace runir::kr::dl::semantics::ext

#endif
