#ifndef RUNIR_KR_PS_UNS_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_PS_UNS_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/ps/uns/dl/syntactic_complexity.hpp"
#include "runir/kr/ps/uns/sketch_view.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <cstddef>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::uns
{

template<typename FeatureTag, typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, FeatureTag>>, C> view)
{
    return ygg::visit([](auto feature) { return runir::kr::ps::uns::dl::syntactic_complexity(feature); }, view.get_variant());
}

template<typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::ps::uns::Sketch>, C> view)
{
    auto result = std::size_t { 0 };
    result += runir::kr::ps::uns::dl::syntactic_complexity(view.get_rules());
    return result;
}

}  // namespace runir::kr::ps::uns

#endif
