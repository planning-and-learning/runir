#ifndef RUNIR_KR_PS_EFFECT_INDEX_HPP_
#define RUNIR_KR_PS_EFFECT_INDEX_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family>
struct Index<runir::kr::ps::EffectVariant<Family>> : IndexMixin<Index<runir::kr::ps::EffectVariant<Family>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::EffectVariant<Family>>>;
    using Base::Base;
};

template<runir::kr::FamilyTag Family, typename LanguageTag>
struct Index<runir::kr::ps::ConcreteEffectVariant<Family, LanguageTag>> : IndexMixin<Index<runir::kr::ps::ConcreteEffectVariant<Family, LanguageTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ConcreteEffectVariant<Family, LanguageTag>>>;
    using Base::Base;
};

template<runir::kr::FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct Index<runir::kr::ps::ConcreteEffect<Family, LanguageTag, FeatureTag, ObservationTag>> :
    IndexMixin<Index<runir::kr::ps::ConcreteEffect<Family, LanguageTag, FeatureTag, ObservationTag>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ConcreteEffect<Family, LanguageTag, FeatureTag, ObservationTag>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
