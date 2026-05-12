#ifndef RUNIR_KR_GP_EFFECT_INDEX_HPP_
#define RUNIR_KR_GP_EFFECT_INDEX_HPP_

#include "runir/kr/gp/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::gp::EffectVariantTag> : IndexMixin<Index<runir::kr::gp::EffectVariantTag>>
{
    using Base = IndexMixin<Index<runir::kr::gp::EffectVariantTag>>;
    using Base::Base;
};

template<typename FeatureTypeTag, typename ChangeTag>
struct Index<runir::kr::gp::EffectTag<FeatureTypeTag, ChangeTag>> : IndexMixin<Index<runir::kr::gp::EffectTag<FeatureTypeTag, ChangeTag>>>
{
    using Base = IndexMixin<Index<runir::kr::gp::EffectTag<FeatureTypeTag, ChangeTag>>>;
    using Base::Base;
};

template<typename LangTag, typename FeatureTypeTag, typename ChangeTag>
struct Index<runir::kr::gp::Effect<LangTag, FeatureTypeTag, ChangeTag>> : IndexMixin<Index<runir::kr::gp::Effect<LangTag, FeatureTypeTag, ChangeTag>>>
{
    using Base = IndexMixin<Index<runir::kr::gp::Effect<LangTag, FeatureTypeTag, ChangeTag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
