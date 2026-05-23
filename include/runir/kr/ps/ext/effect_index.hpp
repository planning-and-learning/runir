#ifndef RUNIR_KR_PS_EXT_EFFECT_INDEX_HPP_
#define RUNIR_KR_PS_EXT_EFFECT_INDEX_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::ps::ext::EffectVariant> : IndexMixin<Index<runir::kr::ps::ext::EffectVariant>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::EffectVariant>>;
    using Base::Base;
};

template<typename LanguageTag>
struct Index<runir::kr::ps::ext::ConcreteEffectVariant<LanguageTag>> : IndexMixin<Index<runir::kr::ps::ext::ConcreteEffectVariant<LanguageTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::ConcreteEffectVariant<LanguageTag>>>;
    using Base::Base;
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct Index<runir::kr::ps::ext::ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>> :
    IndexMixin<Index<runir::kr::ps::ext::ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
