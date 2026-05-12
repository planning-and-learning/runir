#ifndef RUNIR_KR_GP_CONDITION_INDEX_HPP_
#define RUNIR_KR_GP_CONDITION_INDEX_HPP_

#include "runir/kr/gp/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::gp::ConditionVariantTag> : IndexMixin<Index<runir::kr::gp::ConditionVariantTag>>
{
    using Base = IndexMixin<Index<runir::kr::gp::ConditionVariantTag>>;
    using Base::Base;
};

template<typename FeatureTypeTag, typename ObservationTag>
struct Index<runir::kr::gp::ConditionTag<FeatureTypeTag, ObservationTag>> : IndexMixin<Index<runir::kr::gp::ConditionTag<FeatureTypeTag, ObservationTag>>>
{
    using Base = IndexMixin<Index<runir::kr::gp::ConditionTag<FeatureTypeTag, ObservationTag>>>;
    using Base::Base;
};

template<typename LangTag, typename FeatureTypeTag, typename ObservationTag>
struct Index<runir::kr::gp::Condition<LangTag, FeatureTypeTag, ObservationTag>> :
    IndexMixin<Index<runir::kr::gp::Condition<LangTag, FeatureTypeTag, ObservationTag>>>
{
    using Base = IndexMixin<Index<runir::kr::gp::Condition<LangTag, FeatureTypeTag, ObservationTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
