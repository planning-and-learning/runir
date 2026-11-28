#ifndef RUNIR_KR_PS_ICP_PROGRAM_VIEW_HPP_
#define RUNIR_KR_PS_ICP_PROGRAM_VIEW_HPP_

#include "runir/kr/ps/icp/module_view.hpp"
#include "runir/kr/ps/icp/program_data.hpp"

#include <cstddef>

namespace runir::kr::ps::icp
{
/// The syntactic complexity of a program (defined in syntactic_complexity.hpp), declared before its view, whose
/// functions() list it.
template<typename C>
std::size_t syntactic_complexity(const urd::index_view<Program, C>& view);
}  // namespace runir::kr::ps::icp

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::icp::Program, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto entry_mod() const noexcept { return self().template member<&urd::record_of<V>::entry_mod>(); }

    static constexpr auto functions() noexcept
    { return urd::Functions<urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::ps::icp::syntactic_complexity)>> {}; }
};
}  // namespace urd

#endif
