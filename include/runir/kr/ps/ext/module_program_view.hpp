#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_VIEW_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_VIEW_HPP_

#include "runir/kr/ps/ext/module_program_data.hpp"
#include "runir/kr/ps/ext/module_view.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/vector.hpp>

namespace tyr
{

template<typename C>
class View<Index<runir::kr::ps::ext::ModuleProgram>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ext::ModuleProgram> m_handle;

public:
    View(Index<runir::kr::ps::ext::ModuleProgram> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_entry_module() const noexcept { return View<Index<runir::kr::ps::ext::Module>, C>(get_data().entry_module, *m_context); }
    auto get_modules() const noexcept { return make_view(get_data().modules, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
