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
    auto operation() const noexcept { return self().template member<&urd::record_of<V>::operation>(); }

    auto object() const noexcept { return self().template member<&urd::record_of<V>::object>(); }

    auto feature() const noexcept { return self().template member<&urd::record_of<V>::feature>(); }
};
}  // namespace urd

#endif
