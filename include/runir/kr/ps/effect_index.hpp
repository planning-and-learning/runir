#ifndef RUNIR_KR_PS_EFFECT_INDEX_HPP_
#define RUNIR_KR_PS_EFFECT_INDEX_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::ps::EffectVariant> : IndexMixin<Index<runir::kr::ps::EffectVariant>>
{
    using Base = IndexMixin<Index<runir::kr::ps::EffectVariant>>;
    using Base::Base;
};

template<typename LanguageTag>
struct Index<runir::kr::ps::ConcreteEffectVariant<LanguageTag>> : IndexMixin<Index<runir::kr::ps::ConcreteEffectVariant<LanguageTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ConcreteEffectVariant<LanguageTag>>>;
    using Base::Base;
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct Index<runir::kr::ps::ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>> :
    IndexMixin<Index<runir::kr::ps::ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
