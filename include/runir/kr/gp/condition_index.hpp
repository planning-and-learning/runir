#ifndef RUNIR_KR_GP_CONDITION_INDEX_HPP_
#define RUNIR_KR_GP_CONDITION_INDEX_HPP_

#include "runir/kr/gp/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::gp::ConditionVariant> : IndexMixin<Index<runir::kr::gp::ConditionVariant>>
{
    using Base = IndexMixin<Index<runir::kr::gp::ConditionVariant>>;
    using Base::Base;
};

template<typename LanguageTag>
struct Index<runir::kr::gp::ConcreteConditionVariant<LanguageTag>> : IndexMixin<Index<runir::kr::gp::ConcreteConditionVariant<LanguageTag>>>
{
    using Base = IndexMixin<Index<runir::kr::gp::ConcreteConditionVariant<LanguageTag>>>;
    using Base::Base;
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct Index<runir::kr::gp::ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>> :
    IndexMixin<Index<runir::kr::gp::ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>>>
{
    using Base = IndexMixin<Index<runir::kr::gp::ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
