#ifndef RUNIR_KR_PS_BASE_DL_FEATURE_INDEX_HPP_
#define RUNIR_KR_PS_BASE_DL_FEATURE_INDEX_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename FeatureTag>
struct Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>> :
    IndexMixin<Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
