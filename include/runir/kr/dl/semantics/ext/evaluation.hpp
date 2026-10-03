#ifndef RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_HPP_

#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/ext/state_evaluation_context.hpp"

namespace runir::kr::dl::semantics
{

template<StateEvaluationContextConcept<runir::kr::ExtFamilyTag> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConcept<runir::kr::ExtFamilyTag, RegisterTag>>, C> constructor, Context& context) -> DenotationView<ConceptTag>
{
    auto result = detail::make_concept_builder(context);
    auto result_bitset = result->get();

    const auto object = context.registers().at(constructor.get_register().get_identifier());
    if (object)
        result_bitset.set(ygg::uint_t(object.value().get_index()));

    return intern_denotation(result, context.get_builder(), context.get_denotation_repository()).first;
}

template<StateEvaluationContextConcept<runir::kr::ExtFamilyTag> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyRole<runir::kr::ExtFamilyTag, RegisterTag>>, C> constructor, Context& context) -> DenotationView<RoleTag>
{
    auto result = detail::make_role_builder(context);

    const auto value = context.registers().at(constructor.get_register().get_identifier());
    if (value)
    {
        const auto pair = value.value();
        result->get(pair.get_first().get_index()).set(ygg::uint_t(pair.get_second().get_index()));
    }

    return intern_denotation(result, context.get_builder(), context.get_denotation_repository()).first;
}

template<StateEvaluationContextConcept<runir::kr::ExtFamilyTag> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConcept<runir::kr::ExtFamilyTag, ArgumentTag<ConceptTag>>>, C> constructor,
                   Context& context) -> DenotationView<ConceptTag>
{
    return context.arguments().at(constructor.get_argument().get_identifier());
}

template<StateEvaluationContextConcept<runir::kr::ExtFamilyTag> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyRole<runir::kr::ExtFamilyTag, ArgumentTag<RoleTag>>>, C> constructor, Context& context) -> DenotationView<RoleTag>
{
    return context.arguments().at(constructor.get_argument().get_identifier());
}

template<StateEvaluationContextConcept<runir::kr::ExtFamilyTag> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyBoolean<runir::kr::ExtFamilyTag, ArgumentTag<BooleanTag>>>, C> constructor,
                   Context& context) -> DenotationView<BooleanTag>
{
    return context.arguments().at(constructor.get_argument().get_identifier());
}

template<StateEvaluationContextConcept<runir::kr::ExtFamilyTag> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<FamilyNumerical<runir::kr::ExtFamilyTag, ArgumentTag<NumericalTag>>>, C> constructor,
                   Context& context) -> DenotationView<NumericalTag>
{
    return context.arguments().at(constructor.get_argument().get_identifier());
}

}  // namespace runir::kr::dl::semantics

#endif
