#ifndef RUNIR_KR_UNS_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_UNS_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/uns/classifier_view.hpp"
#include "runir/kr/uns/dl/syntactic_complexity.hpp"

#include <cstddef>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::uns
{

template<typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>>, C> view)
{
    return ygg::visit([](auto feature) { return runir::kr::uns::dl::syntactic_complexity(feature); }, view.get_variant());
}

template<typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<Classifier>, C> view)
{
    auto result = std::size_t { 0 };
    for (auto feature : view.get_features())
        result += syntactic_complexity(feature);
    return result;
}

}  // namespace runir::kr::uns

#endif
