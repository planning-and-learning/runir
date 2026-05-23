#ifndef RUNIR_KR_PS_EXT_DL_FEATURE_INDEX_HPP_
#define RUNIR_KR_PS_EXT_DL_FEATURE_INDEX_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename FeatureTag>
struct Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, FeatureTag>> : IndexMixin<Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, FeatureTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, FeatureTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
