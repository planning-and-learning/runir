#ifndef RUNIR_KR_GP_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_GP_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/gp/dl/syntactic_complexity.hpp"
#include "runir/kr/gp/feature_view.hpp"
#include "runir/kr/gp/policy_view.hpp"

#include <cstddef>
#include <tyr/common/types.hpp>

namespace runir::kr::gp
{

template<typename FeatureTag, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::gp::Feature<FeatureTag>>, C> view)
{
    return view.get_variant().apply([](auto feature) { return runir::kr::gp::dl::syntactic_complexity(feature); });
}

template<typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::gp::Policy>, C> view)
{
    auto result = std::size_t { 0 };
    result += runir::kr::gp::dl::syntactic_complexity(view.get_rules());
    return result;
}

}  // namespace runir::kr::gp

#endif
