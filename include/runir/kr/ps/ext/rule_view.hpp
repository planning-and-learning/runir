#ifndef RUNIR_KR_PS_EXT_RULE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_RULE_VIEW_HPP_

#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/ps/ext/dl/feature_view.hpp"
#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/module_symbol_view.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::ps::ext::RuleKind Kind, typename C>
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
        requires(std::same_as<Kind, runir::kr::ps::ext::SketchTag> || std::same_as<Kind, runir::kr::ps::ext::DoTag>)
    {
        return make_view(get_data().effects, *m_context);
    }

    auto get_feature() const noexcept
        requires(std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::ConceptTag>>
                 || std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::RoleTag>>)
    {
        return make_view(get_data().feature, *m_context);
    }

    auto get_register() const noexcept
        requires(std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::ConceptTag>>
                 || std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::RoleTag>>)
    {
        return make_view(get_data().reg, get_repository(*m_context).get_dl_repository());
    }

    const auto& get_action_name() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::DoTag>
    {
        return get_data().action_name;
    }

    auto get_action_arguments() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::DoTag>
    {
        return make_view(get_data().arguments, *m_context);
    }

    auto get_callee() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::CallTag>
    {
        return View<Index<runir::kr::ps::ext::ModuleSymbol>, C>(get_data().callee, *m_context);
    }

    template<typename F>
    void for_each_call_argument(F&& function) const
        requires std::same_as<Kind, runir::kr::ps::ext::CallTag>
    {
        for (const auto& argument : get_data().arguments)
            argument.apply([&](auto feature) { function(make_view(feature, *m_context)); });
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
