#ifndef RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_HPP_

#include "runir/kr/dl/ext/concept_data.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/ext/evaluation_context.hpp"

namespace runir::kr::dl::semantics
{

template<tyr::planning::TaskKind Kind, typename C>
auto evaluate_impl(tyr::View<tyr::Index<FamilyConcept<ExtFamilyTag, RegisterTag>>, C> constructor,
                   EvaluationContext<ExtFamilyTag, Kind>& context,
                   EvaluationWorkspace&) -> decltype(detail::make_concept_builder(context))
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
                   EvaluationWorkspace&) -> decltype(detail::make_role_builder(context))
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

}  // namespace runir::kr::dl::semantics

namespace runir::kr::dl::semantics::ext
{

using FamilyTag = runir::kr::ExtFamilyTag;

template<tyr::planning::TaskKind Kind>
using EvaluationContext = runir::kr::dl::semantics::EvaluationContext<FamilyTag, Kind>;

template<tyr::planning::TaskKind Kind>
const auto& get_repository(const EvaluationContext<Kind>& context) noexcept
{
    return runir::kr::dl::semantics::get_repository(context);
}

using runir::kr::dl::semantics::evaluate;
using runir::kr::dl::semantics::evaluate_impl;

}  // namespace runir::kr::dl::semantics::ext

#endif
