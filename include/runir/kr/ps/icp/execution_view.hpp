#ifndef RUNIR_KR_PS_ICP_EXECUTION_VIEW_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_VIEW_HPP_

#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/ps/icp/execution_repository.hpp"

#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::ps::icp::Histories> C>
class View<Index<runir::kr::ps::icp::Histories>, C> : public ygg::IndexViewBase<runir::kr::ps::icp::Histories, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::icp::Histories, C>::IndexViewBase;

    auto get_concepts() const { return make_view(this->get_data().concepts, this->get_context().get_denotation_repository()); }
};

template<tyr::TaskKind Kind, formalism::SymbolContextFor<runir::kr::ps::icp::ProgramState<Kind>> C>
class View<Index<runir::kr::ps::icp::ProgramState<Kind>>, C> : public ygg::IndexViewBase<runir::kr::ps::icp::ProgramState<Kind>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::icp::ProgramState<Kind>, C>::IndexViewBase;

    auto get_program() const { return make_view(this->get_data().program, this->get_context().get_program_repository()); }
    auto get_module() const { return get_program().get_module(); }
    auto get_memory_state() const { return make_view(this->get_data().memory_state, this->get_context().get_program_repository()); }
    auto get_registers() const { return make_view(this->get_data().registers, this->get_context().get_denotation_repository()); }
    auto get_histories() const { return make_view(this->get_data().histories, this->get_context()); }
    auto get_state() const { return this->get_context().get_state_repository().get_registered_state(this->get_data().state); }
};

}

#endif
