#ifndef RUNIR_KR_PS_DL_EFFECT_VIEW_HPP_
#define RUNIR_KR_PS_DL_EFFECT_VIEW_HPP_

#include "runir/kr/ps/dl/effect_data.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, runir::kr::ps::dl::EffectObservationTag<FeatureTag> ObservationTag, typename C>
class View<Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> : public ygg::IndexViewBase<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>, C>::IndexViewBase;

    auto get_feature() const noexcept { return make_view(this->get_data().feature, this->get_context()); }
};

}  // namespace ygg

#endif
