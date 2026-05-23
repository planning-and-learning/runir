#ifndef RUNIR_KR_PS_FEATURE_INDEX_HPP_
#define RUNIR_KR_PS_FEATURE_INDEX_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::FamilyTag Family, typename FeatureTag>
struct Index<runir::kr::ps::Feature<Family, FeatureTag>> : IndexMixin<Index<runir::kr::ps::Feature<Family, FeatureTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::Feature<Family, FeatureTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
