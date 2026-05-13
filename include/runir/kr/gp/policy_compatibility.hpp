#ifndef RUNIR_KR_GP_POLICY_COMPATIBILITY_HPP_
#define RUNIR_KR_GP_POLICY_COMPATIBILITY_HPP_

#include "runir/kr/gp/declarations.hpp"

#include <concepts>
#include <tyr/common/types.hpp>

namespace runir::kr::gp
{

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsPolicyView =
    IsEvaluationContext<LanguageTag, EvaluationContext> && requires(tyr::View<tyr::Index<Policy>, StorageContext> policy, EvaluationContext& context) {
        { policy.is_compatible_with(context) } -> std::same_as<bool>;
    };

}  // namespace runir::kr::gp

#endif
