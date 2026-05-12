#ifndef RUNIR_KR_GP_DL_FEATURE_INDEX_HPP_
#define RUNIR_KR_GP_DL_FEATURE_INDEX_HPP_

#include "runir/kr/gp/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename FeatureTag>
struct Index<runir::kr::gp::ConcreteFeature<runir::kr::DlTag, FeatureTag>> : IndexMixin<Index<runir::kr::gp::ConcreteFeature<runir::kr::DlTag, FeatureTag>>>
{
    using Base = IndexMixin<Index<runir::kr::gp::ConcreteFeature<runir::kr::DlTag, FeatureTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
