#ifndef RUNIR_KR_PS_EXT_EFFECT_INDEX_HPP_
#define RUNIR_KR_PS_EXT_EFFECT_INDEX_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
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

}  // namespace ygg

#endif
