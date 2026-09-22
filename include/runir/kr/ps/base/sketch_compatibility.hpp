#ifndef RUNIR_KR_PS_BASE_SKETCH_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_COMPATIBILITY_HPP_

#include "runir/kr/ps/base/compatibility.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <concepts>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::base
{

template<typename LanguageTag, typename Context, typename StorageContext>
concept IsSketchView = runir::kr::ps::IsTransitionEvaluationContext<runir::kr::BaseFamilyTag, LanguageTag, Context>
                       && requires(ygg::View<ygg::Index<runir::kr::ps::base::Sketch>, StorageContext> sketch, Context& context) {
                              { runir::kr::ps::base::is_compatible_with(sketch, context) } -> std::same_as<bool>;
                          };

}  // namespace runir::kr::ps::base

#endif
