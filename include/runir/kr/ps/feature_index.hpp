#ifndef RUNIR_KR_PS_FEATURE_INDEX_HPP_
#define RUNIR_KR_PS_FEATURE_INDEX_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family, typename FeatureTag>
struct Index<runir::kr::ps::Feature<Family, FeatureTag>> : IndexMixin<Index<runir::kr::ps::Feature<Family, FeatureTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::Feature<Family, FeatureTag>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
