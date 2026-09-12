#ifndef RUNIR_KR_UNS_DL_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_UNS_DL_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/dl/semantics/syntactic_complexity.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"

#include <cstddef>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::uns::dl
{

template<typename C>
std::size_t syntactic_complexity(
    ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>, C> view)
{
    return runir::kr::dl::semantics::syntactic_complexity(view.get_feature());
}

}  // namespace runir::kr::uns::dl

#endif
