#ifndef RUNIR_KR_PS_ICP_PROGRAM_VIEW_HPP_
#define RUNIR_KR_PS_ICP_PROGRAM_VIEW_HPP_

#include "runir/kr/ps/icp/module_view.hpp"
#include "runir/kr/ps/icp/program_data.hpp"

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::icp::Program, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_module() const noexcept { return self().template member<&urd::record_of<V>::module>(); }
    auto get_entry_module() const noexcept { return get_module(); }
};
}  // namespace urd

#endif
