#ifndef RUNIR_KR_PS_DL_CONDITION_VIEW_HPP_
#define RUNIR_KR_PS_DL_CONDITION_VIEW_HPP_

#include "runir/kr/ps/dl/condition_data.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<runir::kr::FamilyTag Family,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         runir::kr::ps::dl::ConditionObservationTag<FeatureTag> ObservationTag,
         typename V>
struct view_mixin<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_feature() const noexcept { return self().template member<&urd::record_of<V>::feature>(); }
};
}  // namespace urd

#endif
