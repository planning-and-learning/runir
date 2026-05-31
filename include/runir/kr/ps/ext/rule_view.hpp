#ifndef RUNIR_KR_PS_EXT_RULE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_RULE_VIEW_HPP_

#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/register_view.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"

#include <string>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/containers/vector.hpp>

namespace ygg
{

template<typename Kind, typename C>
class View<Index<runir::kr::ps::ext::Rule<Kind>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ext::Rule<Kind>> m_handle;

public:
    View(Index<runir::kr::ps::ext::Rule<Kind>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_source() const noexcept { return View<Index<runir::kr::ps::ext::MemoryState>, C>(get_data().source, *m_context); }
    auto get_target() const noexcept { return View<Index<runir::kr::ps::ext::MemoryState>, C>(get_data().target, *m_context); }
    auto get_conditions() const noexcept { return make_view(get_data().conditions, *m_context); }

    auto get_effects() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::SketchTag>
    {
        return make_view(get_data().effects, *m_context);
    }

    auto get_concept() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::LoadTag>
    {
        return make_view(get_data().load_concept, get_repository(*m_context).get_dl_repository());
    }

    auto get_register() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::LoadTag>
    {
        return View<Index<runir::kr::ps::ext::Register>, C>(get_data().reg, *m_context);
    }

    const auto& get_action_name() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::DoTag>
    {
        return get_data().action_name;
    }

    auto get_action_arguments() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::DoTag>
    {
        return make_view(get_data().arguments, get_repository(*m_context).get_dl_repository());
    }

    auto get_callee() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::CallTag>
    {
        return View<Index<runir::kr::ps::ext::Module>, C>(get_data().callee, *m_context);
    }

    std::string get_callee_name() const
        requires std::same_as<Kind, runir::kr::ps::ext::CallTag>
    {
        if (get_data().callee_name.size() != 0)
            return std::string(get_data().callee_name);
        return std::string(get_callee().get_name());
    }

    const auto& get_call_arguments() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::CallTag>
    {
        return get_data().arguments;
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
