#ifndef RUNIR_KR_PS_CONDITION_INDEX_HPP_
#define RUNIR_KR_PS_CONDITION_INDEX_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::ps::ConditionVariant> : IndexMixin<Index<runir::kr::ps::ConditionVariant>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ConditionVariant>>;
    using Base::Base;
};

template<typename LanguageTag>
struct Index<runir::kr::ps::ConcreteConditionVariant<LanguageTag>> : IndexMixin<Index<runir::kr::ps::ConcreteConditionVariant<LanguageTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ConcreteConditionVariant<LanguageTag>>>;
    using Base::Base;
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct Index<runir::kr::ps::ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>> :
    IndexMixin<Index<runir::kr::ps::ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
