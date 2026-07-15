#ifndef RUNIR_KR_PS_EXT_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_PS_EXT_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/ps/ext/dl/syntactic_complexity.hpp"
#include "runir/kr/ps/ext/module_program_view.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <cstddef>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext
{

template<typename FeatureTag, typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> view)
{
    return ygg::visit([](auto feature) { return runir::kr::ps::ext::dl::syntactic_complexity(feature); }, view.get_variant());
}

template<typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<Module>, C> view)
{
    auto result = std::size_t { 0 };
    for (auto feature : view.template get_features<runir::kr::dl::ConceptTag>())
        result += syntactic_complexity(feature);
    for (auto feature : view.template get_features<runir::kr::dl::RoleTag>())
        result += syntactic_complexity(feature);
    for (auto feature : view.template get_features<runir::kr::ps::dl::BooleanFeature>())
        result += syntactic_complexity(feature);
    for (auto feature : view.template get_features<runir::kr::ps::dl::NumericalFeature>())
        result += syntactic_complexity(feature);
    return result;
}

template<typename C>
std::size_t syntactic_complexity(ygg::View<ygg::Index<ModuleProgram>, C> view)
{
    auto result = std::size_t { 0 };
    for (auto module : view.get_modules())
        result += syntactic_complexity(module);
    return result;
}

}  // namespace runir::kr::ps::ext

#endif
