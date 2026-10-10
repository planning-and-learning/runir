#ifndef RUNIR_KR_PS_EXT_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_PS_EXT_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/syntactic_complexity.hpp"

#include <cstddef>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext
{

template<typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<Module>, C> view)
{
    auto result = std::size_t { 0 };
    for (auto feature : view.template get_features<runir::kr::dl::ConceptTag>())
        result += runir::kr::ps::syntactic_complexity(feature);
    for (auto feature : view.template get_features<runir::kr::dl::RoleTag>())
        result += runir::kr::ps::syntactic_complexity(feature);
    for (auto feature : view.template get_features<runir::kr::dl::BooleanTag>())
        result += runir::kr::ps::syntactic_complexity(feature);
    for (auto feature : view.template get_features<runir::kr::dl::NumericalTag>())
        result += runir::kr::ps::syntactic_complexity(feature);
    for (auto feature : view.get_query_features())
        result += runir::kr::ps::syntactic_complexity(feature);
    return result;
}

template<typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<Program>, C> view)
{
    auto result = std::size_t { 0 };
    for (auto module_ : view.get_modules())
        result += syntactic_complexity(module_);
    return result;
}

}  // namespace runir::kr::ps::ext

#endif
