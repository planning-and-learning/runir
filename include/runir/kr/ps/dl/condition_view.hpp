#ifndef RUNIR_KR_PS_DL_CONDITION_VIEW_HPP_
#define RUNIR_KR_PS_DL_CONDITION_VIEW_HPP_

#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/dl/condition_data.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         runir::kr::ps::dl::ConditionObservationTag<FeatureTag> ObservationTag,
         typename C>
class View<Index<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> : public ygg::IndexViewBase<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>, C>::IndexViewBase;

    auto get_feature() const noexcept { return make_view(this->get_data().feature, this->get_context()); }
};

}  // namespace ygg

#endif
