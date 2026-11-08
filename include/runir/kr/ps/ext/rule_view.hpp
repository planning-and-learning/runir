#ifndef RUNIR_KR_PS_EXT_RULE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_RULE_VIEW_HPP_

#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/memory_state_view.hpp"
#include "runir/kr/ps/module_symbol_view.hpp"
#include "runir/kr/ps/ext/order_term_view.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <concepts>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<runir::kr::ps::ext::RuleKind Kind, typename V>
struct view_mixin<runir::kr::ps::ext::Rule<Kind>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_source() const noexcept { return self().template member<&urd::record_of<V>::source>(); }
    auto get_target() const noexcept
        requires(!std::same_as<Kind, runir::kr::ps::ext::BacktrackTag>)
    { return self().template member<&urd::record_of<V>::target>(); }
    auto get_conditions() const noexcept { return self().template member<&urd::record_of<V>::conditions>(); }

    auto get_effects() const noexcept
        requires(runir::kr::ps::ext::BindingRuleKind<Kind> || std::same_as<Kind, runir::kr::ps::ext::SketchTag> || std::same_as<Kind, runir::kr::ps::ext::DoTag>
                 || std::same_as<Kind, runir::kr::ps::ext::ActionTag>)
    { return self().template member<&urd::record_of<V>::effects>(); }

    auto get_feature() const noexcept
        requires runir::kr::ps::ext::BindingRuleKind<Kind>
    { return self().template member<&urd::record_of<V>::feature>(); }

    auto get_register() const noexcept
        requires runir::kr::ps::ext::BindingRuleKind<Kind>
    { return urd::resolve(self().data().reg, get_repository(self().context()).get_dl_repository()); }

    auto get_order() const noexcept
        requires(std::same_as<Kind, runir::kr::ps::ext::ChooseTag<runir::kr::dl::ConceptTag>>
                 || std::same_as<Kind, runir::kr::ps::ext::ChooseTag<runir::kr::dl::RoleTag>>)
    { return self().template member<&urd::record_of<V>::order>(); }

    const auto& get_action_name() const noexcept
        requires(std::same_as<Kind, runir::kr::ps::ext::DoTag> || std::same_as<Kind, runir::kr::ps::ext::ActionTag>)
    { return self().data().action_name; }

    auto get_action_arguments() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::DoTag>
    { return self().template member<&urd::record_of<V>::arguments>(); }

    auto get_query_feature() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::ActionTag>
    { return self().template member<&urd::record_of<V>::query_feature>(); }

    auto get_callee() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::CallTag>
    { return self().template member<&urd::record_of<V>::callee>(); }

    auto get_call_arguments() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::CallTag>
    { return self().template member<&urd::record_of<V>::arguments>(); }

    template<typename F>
    void for_each_call_argument(F function) const
        requires std::same_as<Kind, runir::kr::ps::ext::CallTag>
    {
        for (const auto& argument : self().data().arguments)
            argument.apply([&](auto feature) { function(urd::resolve(feature, self().context())); });
    }
};
}  // namespace urd

#endif
