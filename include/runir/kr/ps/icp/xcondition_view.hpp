#ifndef RUNIR_KR_PS_ICP_XCONDITION_VIEW_HPP_
#define RUNIR_KR_PS_ICP_XCONDITION_VIEW_HPP_

#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/icp/xcondition_data.hpp"

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::icp::XCondition, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_operation() const noexcept { return self().data().operation; }
    const auto& get_object_reference() const noexcept { return self().data().object; }
    auto get_concept_feature() const noexcept { return self().template get<&urd::record_of<V>::feature>(); }
    auto get_feature() const noexcept { return get_concept_feature(); }
};
}  // namespace urd

#endif
