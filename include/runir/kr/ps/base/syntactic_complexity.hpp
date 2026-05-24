#ifndef RUNIR_KR_PS_BASE_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_PS_BASE_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/ps/base/dl/syntactic_complexity.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/sketch_view.hpp"

#include <cstddef>
#include <tyr/common/types.hpp>

namespace runir::kr::ps::base
{

template<typename FeatureTag, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>, C> view)
{
    return tyr::visit([](auto feature) { return runir::kr::ps::base::dl::syntactic_complexity(feature); }, view.get_variant());
}

template<typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::ps::Sketch<runir::kr::BaseFamilyTag>>, C> view)
{
    auto result = std::size_t { 0 };
    result += runir::kr::ps::base::dl::syntactic_complexity(view.get_rules());
    return result;
}

}  // namespace runir::kr::ps::base

#endif
