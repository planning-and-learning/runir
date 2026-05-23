#ifndef RUNIR_KR_PS_SKETCH_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_SKETCH_COMPATIBILITY_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <concepts>
#include <tyr/common/types.hpp>

namespace runir::kr::ps
{

template<typename Family, typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsSketchView = IsEvaluationContext<Family, LanguageTag, EvaluationContext>
                       && requires(tyr::View<tyr::Index<Sketch<Family>>, StorageContext> sketch, EvaluationContext& context) {
                              { sketch.is_compatible_with(context) } -> std::same_as<bool>;
                          };

}  // namespace runir::kr::ps

#endif
