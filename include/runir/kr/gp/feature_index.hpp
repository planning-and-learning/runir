#ifndef RUNIR_KR_GP_FEATURE_INDEX_HPP_
#define RUNIR_KR_GP_FEATURE_INDEX_HPP_

#include "runir/kr/gp/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename FeatureTypeTag>
struct Index<runir::kr::gp::FeatureTag<FeatureTypeTag>> : IndexMixin<Index<runir::kr::gp::FeatureTag<FeatureTypeTag>>>
{
    using Base = IndexMixin<Index<runir::kr::gp::FeatureTag<FeatureTypeTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
