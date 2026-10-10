#ifndef RUNIR_KR_PS_EXT_RULE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_RULE_VIEW_HPP_

#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/module_symbol_view.hpp"
#include "runir/kr/ps/ext/order_term_view.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::ps::ext::RuleKind Kind, formalism::SymbolContextFor<runir::kr::ps::ext::Rule<Kind>> C>
class View<Index<runir::kr::ps::ext::Rule<Kind>>, C> : public ygg::IndexViewBase<runir::kr::ps::ext::Rule<Kind>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::ext::Rule<Kind>, C>::IndexViewBase;

    auto get_source() const noexcept { return View<Index<runir::kr::ps::ext::MemoryState>, C>(this->get_data().source, this->get_context()); }
    auto get_target() const noexcept
        requires(!std::same_as<Kind, runir::kr::ps::ext::BacktrackTag>)
    {
        return View<Index<runir::kr::ps::ext::MemoryState>, C>(this->get_data().target, this->get_context());
    }
    auto get_conditions() const noexcept { return make_view(this->get_data().conditions, this->get_context()); }

    auto get_effects() const noexcept
        requires(runir::kr::ps::ext::BindingRuleKind<Kind> || std::same_as<Kind, runir::kr::ps::ext::SketchTag> || std::same_as<Kind, runir::kr::ps::ext::DoTag>
                 || std::same_as<Kind, runir::kr::ps::ext::ActionTag>)
    {
        return make_view(this->get_data().effects, this->get_context());
    }

    auto get_feature() const noexcept
        requires runir::kr::ps::ext::BindingRuleKind<Kind>
    {
        return make_view(this->get_data().feature, this->get_context());
    }

    auto get_register() const noexcept
        requires runir::kr::ps::ext::BindingRuleKind<Kind>
    {
        return make_view(this->get_data().reg, this->get_repository().get_dl_repository());
    }

    auto get_order() const noexcept
        requires(std::same_as<Kind, runir::kr::ps::ext::ChooseTag<runir::kr::dl::ConceptTag>>
                 || std::same_as<Kind, runir::kr::ps::ext::ChooseTag<runir::kr::dl::RoleTag>>)
    {
        return make_view(this->get_data().order, this->get_context());
    }

    const auto& get_action_name() const noexcept
        requires(std::same_as<Kind, runir::kr::ps::ext::DoTag> || std::same_as<Kind, runir::kr::ps::ext::ActionTag>)
    {
        return this->get_data().action_name;
    }

    auto get_action_arguments() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::DoTag>
    {
        return make_view(this->get_data().arguments, this->get_context());
    }

    auto get_query_feature() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::ActionTag>
    {
        return make_view(this->get_data().query_feature, this->get_context());
    }

    auto get_callee() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::CallTag>
    {
        return View<Index<runir::kr::ps::ext::ModuleSymbol>, C>(this->get_data().callee, this->get_context());
    }

    auto get_call_arguments() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::CallTag>
    {
        return make_view(this->get_data().arguments, this->get_context());
    }

    template<typename F>
    void for_each_call_argument(F&& function) const
        requires std::same_as<Kind, runir::kr::ps::ext::CallTag>
    {
        for (const auto& argument : this->get_data().arguments)
            argument.apply([&](auto feature) { function(make_view(feature, this->get_context())); });
    }
};

}  // namespace ygg

#endif
