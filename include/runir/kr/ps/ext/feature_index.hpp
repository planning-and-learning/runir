#ifndef RUNIR_KR_PS_EXT_FEATURE_INDEX_HPP_
#define RUNIR_KR_PS_EXT_FEATURE_INDEX_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<typename FeatureTag>
struct Index<runir::kr::ps::ext::Feature<FeatureTag>> : IndexMixin<Index<runir::kr::ps::ext::Feature<FeatureTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::Feature<FeatureTag>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
