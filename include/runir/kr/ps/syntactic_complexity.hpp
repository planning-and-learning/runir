#ifndef RUNIR_KR_PS_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_PS_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/dl/semantics/syntactic_complexity.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"

namespace runir::kr::ps
{

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
std::size_t syntactic_complexity(const urd::index_view<ConcreteFeature<Family, FeatureTag>, C>& view)
{ return runir::kr::dl::semantics::syntactic_complexity(view.feature()); }

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
std::size_t syntactic_complexity(const urd::index_view<Feature<Family, FeatureTag>, C>& view)
{
    return std::visit([](auto feature) { return syntactic_complexity(feature); }, view.variant());
}

}  // namespace runir::kr::ps

#endif
