#ifndef RUNIR_KR_PS_ICP_EXECUTION_VIEW_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_VIEW_HPP_

#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/ps/execution_repository.hpp"
#include "runir/kr/ps/icp/execution_data.hpp"
#include "runir/kr/ps/icp/repository.hpp"

#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::icp::Histories, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_concepts() const { return urd::resolve(self().data().concepts, self().context().get_denotation_repository()); }
};

template<tyr::TaskKind Kind, typename V>
struct view_mixin<runir::kr::ps::icp::ProgramState<Kind>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_program() const { return urd::resolve(self().data().program, self().context().get_program_repository()); }
    auto get_module() const { return get_program().get_module(); }
    auto get_memory_state() const { return urd::resolve(self().data().memory_state, self().context().get_program_repository()); }
    auto get_registers() const { return urd::resolve(self().data().registers, self().context().get_denotation_repository()); }
    auto get_histories() const { return self().template member<&urd::record_of<V>::histories>(); }
    auto get_state() const { return self().context().get_state_repository().get_registered_state(self().data().state); }
};
}  // namespace urd

#endif
