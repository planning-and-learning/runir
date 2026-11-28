#ifndef RUNIR_KR_PS_ICP_CONTROL_VIEW_HPP_
#define RUNIR_KR_PS_ICP_CONTROL_VIEW_HPP_

#include "runir/kr/ps/icp/execution_data.hpp"

#include <urd/elements/view.hpp>

/// The views of the control records (no task kind): declared before any builder or view names one of their children.

namespace urd
{
/// Getters shared by the views of a program's histories.
template<typename V>
struct view_mixin<runir::kr::ps::icp::Histories, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto concepts() const { return self().template member<&urd::record_of<V>::concepts>(); }
};
}

#endif
