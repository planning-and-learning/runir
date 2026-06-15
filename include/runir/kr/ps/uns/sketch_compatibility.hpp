#ifndef RUNIR_KR_PS_UNS_SKETCH_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_UNS_SKETCH_COMPATIBILITY_HPP_

#include "runir/kr/ps/uns/compatibility.hpp"
#include "runir/kr/ps/uns/sketch_view.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <concepts>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::uns
{

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsSketchView = runir::kr::ps::IsEvaluationContext<runir::kr::UnsFamilyTag, LanguageTag, EvaluationContext>
                       && requires(ygg::View<ygg::Index<runir::kr::ps::uns::Sketch>, StorageContext> sketch, EvaluationContext& context) {
                              { runir::kr::ps::uns::is_compatible_with(sketch, context) } -> std::same_as<bool>;
                          };

}  // namespace runir::kr::ps::uns

#endif
