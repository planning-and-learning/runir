#ifndef RUNIR_KR_PS_EXT_CONDITION_INDEX_HPP_
#define RUNIR_KR_PS_EXT_CONDITION_INDEX_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::ps::ext::ConditionVariant> : IndexMixin<Index<runir::kr::ps::ext::ConditionVariant>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::ConditionVariant>>;
    using Base::Base;
};

template<typename LanguageTag>
struct Index<runir::kr::ps::ext::ConcreteConditionVariant<LanguageTag>> : IndexMixin<Index<runir::kr::ps::ext::ConcreteConditionVariant<LanguageTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::ConcreteConditionVariant<LanguageTag>>>;
    using Base::Base;
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct Index<runir::kr::ps::ext::ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>> :
    IndexMixin<Index<runir::kr::ps::ext::ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
