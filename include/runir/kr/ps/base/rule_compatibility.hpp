#ifndef RUNIR_KR_PS_BASE_RULE_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_BASE_RULE_COMPATIBILITY_HPP_

#include "runir/kr/ps/base/compatibility.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <concepts>
#include <tyr/common/types.hpp>

namespace runir::kr::ps::base
{

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsRuleView = runir::kr::ps::IsEvaluationContext<runir::kr::BaseFamilyTag, LanguageTag, EvaluationContext>
                     && requires(tyr::View<tyr::Index<runir::kr::ps::base::Rule>, StorageContext> rule, EvaluationContext& context) {
                            { runir::kr::ps::base::is_compatible_with(rule, context) } -> std::same_as<bool>;
                        };

}  // namespace runir::kr::ps::base

#endif
