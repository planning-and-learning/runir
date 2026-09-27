#ifndef RUNIR_KR_PS_ICP_EXECUTION_VIEW_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_VIEW_HPP_

#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/ps/icp/execution_repository.hpp"

#include <yggdrasil/containers/vector.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::ps::icp::Histories>, C>
{
    Index<runir::kr::ps::icp::Histories> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::ps::icp::Histories> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const { return (*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_concepts() const { return make_view(get_data().concepts, m_context->get_denotation_repository()); }

    auto identifying_members() const noexcept { return std::make_tuple(m_handle, m_context->get_index()); }
};

template<tyr::TaskKind Kind, typename C>
class View<Index<runir::kr::ps::icp::ProgramState<Kind>>, C>
{
    Index<runir::kr::ps::icp::ProgramState<Kind>> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::ps::icp::ProgramState<Kind>> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const { return (*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_program() const { return make_view(get_data().program, m_context->get_program_repository()); }
    auto get_module() const { return get_program().get_module(); }
    auto get_memory_state() const { return make_view(get_data().memory_state, m_context->get_program_repository()); }
    auto get_registers() const { return make_view(get_data().registers, m_context->get_denotation_repository()); }
    auto get_histories() const { return make_view(get_data().histories, *m_context); }
    auto get_state() const { return m_context->get_state_repository().get_registered_state(get_data().state); }

    auto identifying_members() const noexcept { return std::make_tuple(m_handle, m_context->get_index()); }
};

}

#endif
