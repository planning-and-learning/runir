#ifndef RUNIR_KR_PS_ICP_PROGRAM_VIEW_HPP_
#define RUNIR_KR_PS_ICP_PROGRAM_VIEW_HPP_

#include "runir/kr/ps/icp/module_view.hpp"
#include "runir/kr/ps/icp/program_data.hpp"

namespace ygg
{

template<typename C>
class View<Index<runir::kr::ps::icp::Program>, C>
{
    const C* m_context;
    Index<runir::kr::ps::icp::Program> m_handle;

public:
    View(Index<runir::kr::ps::icp::Program> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_module() const noexcept { return make_view(get_data().module, *m_context); }
    auto get_entry_module() const noexcept { return get_module(); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}

#endif
