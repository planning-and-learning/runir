#ifndef RUNIR_KR_MPG_DL_FEATURE_INDEX_HPP_
#define RUNIR_KR_MPG_DL_FEATURE_INDEX_HPP_

#include "runir/kr/mpg/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename FeatureTag>
struct Index<runir::kr::mpg::ConcreteFeature<runir::kr::DlTag, FeatureTag>> : IndexMixin<Index<runir::kr::mpg::ConcreteFeature<runir::kr::DlTag, FeatureTag>>>
{
    using Base = IndexMixin<Index<runir::kr::mpg::ConcreteFeature<runir::kr::DlTag, FeatureTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
