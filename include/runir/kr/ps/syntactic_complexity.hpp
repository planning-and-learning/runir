#ifndef RUNIR_KR_PS_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_PS_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/ps/dl/syntactic_complexity.hpp"
#include "runir/kr/ps/feature_view.hpp"

namespace runir::kr::ps
{

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<Feature<Family, FeatureTag>>, C> view)
{
    return ygg::visit([](auto feature) { return runir::kr::ps::dl::syntactic_complexity(feature); }, view.get_variant());
}

}  // namespace runir::kr::ps

#endif
