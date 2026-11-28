#ifndef RUNIR_KR_PS_ICP_EXECUTION_VIEW_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_VIEW_HPP_

#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/ps/execution_repository.hpp"
#include "runir/kr/ps/icp/control_view.hpp"
#include "runir/kr/ps/icp/execution_data.hpp"
#include "runir/kr/ps/icp/repository.hpp"

#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<tyr::TaskKind Kind, typename V>
struct view_mixin<runir::kr::ps::icp::ProgramState<Kind>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto program() const { return self().template member<&urd::record_of<V>::program>(); }

    auto memory_state() const { return self().template member<&urd::record_of<V>::memory_state>(); }

    auto registers() const { return self().template member<&urd::record_of<V>::registers>(); }

    auto histories() const { return self().template member<&urd::record_of<V>::histories>(); }

    auto state() const { return self().template member<&urd::record_of<V>::state>().unpack(); }
};
}  // namespace urd

#endif
