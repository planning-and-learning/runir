#ifndef RUNIR_KR_PS_UNS_DL_FEATURE_INDEX_HPP_
#define RUNIR_KR_PS_UNS_DL_FEATURE_INDEX_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<typename FeatureTag>
struct Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, FeatureTag>> :
    IndexMixin<Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, FeatureTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, FeatureTag>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
