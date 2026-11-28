#ifndef RUNIR_KR_PS_ICP_RESET_PAIR_VIEW_HPP_
#define RUNIR_KR_PS_ICP_RESET_PAIR_VIEW_HPP_

#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/icp/reset_pair_data.hpp"

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::icp::ResetPair, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto before() const noexcept { return self().template member<&urd::record_of<V>::before>(); }

    auto after() const noexcept { return self().template member<&urd::record_of<V>::after>(); }
};
}  // namespace urd

#endif
