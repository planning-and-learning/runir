#ifndef RUNIR_KR_PS_EXT_ORDER_TERM_VIEW_HPP_
#define RUNIR_KR_PS_EXT_ORDER_TERM_VIEW_HPP_

#include "runir/kr/ps/ext/order_term_data.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <urd/elements/flat.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::ext::OrderTerm, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_direction() const noexcept { return self().data().direction; }
    auto get_feature() const noexcept { return self().template member<&urd::record_of<V>::feature>(); }
};
}  // namespace urd

#endif
