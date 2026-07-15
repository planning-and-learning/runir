#ifndef RUNIR_KR_PS_EXT_DL_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_PS_EXT_DL_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/dl/semantics/syntactic_complexity.hpp"
#include "runir/kr/ps/ext/dl/feature_view.hpp"

#include <cstddef>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext::dl
{

template<typename FeatureTag, typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view)
{
    return 1 + runir::kr::dl::semantics::syntactic_complexity(view.get_feature());
}

}  // namespace runir::kr::ps::ext::dl

#endif
