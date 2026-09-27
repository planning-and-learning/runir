#ifndef RUNIR_KR_PS_DL_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_PS_DL_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/dl/semantics/syntactic_complexity.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"

#include <cstddef>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::dl
{

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>, C> view)
{
    return runir::kr::dl::semantics::syntactic_complexity(view.get_feature());
}

}  // namespace runir::kr::ps::dl

#endif
