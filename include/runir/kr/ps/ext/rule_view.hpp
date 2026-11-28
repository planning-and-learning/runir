#ifndef RUNIR_KR_PS_EXT_RULE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_RULE_VIEW_HPP_

#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/ext/order_term_view.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/memory_state_view.hpp"
#include "runir/kr/ps/module_symbol_view.hpp"

#include <concepts>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <variant>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<runir::kr::ps::ext::RuleKind Kind, typename V>
struct view_mixin<runir::kr::ps::ext::Rule<Kind>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto source() const noexcept { return self().template member<&urd::record_of<V>::source>(); }

    auto target() const noexcept
        requires runir::kr::ps::ext::TransitionRuleKind<Kind>
    { return self().template member<&urd::record_of<V>::target>(); }

    auto conditions() const noexcept { return self().template member<&urd::record_of<V>::conditions>(); }

    auto feature() const noexcept
        requires runir::kr::ps::ext::BindingRuleKind<Kind>
    { return self().template member<&urd::record_of<V>::feature>(); }

    auto slot() const noexcept
        requires runir::kr::ps::ext::BindingRuleKind<Kind>
    { return self().template member<&urd::record_of<V>::slot>(); }

    auto effects() const noexcept
        requires runir::kr::ps::ext::EffectRuleKind<Kind>
    { return self().template member<&urd::record_of<V>::effects>(); }

    auto order() const noexcept
        requires(std::same_as<Kind, runir::kr::ps::ext::ChooseTag<runir::kr::dl::ConceptTag>>
                 || std::same_as<Kind, runir::kr::ps::ext::ChooseTag<runir::kr::dl::RoleTag>>)
    { return self().template member<&urd::record_of<V>::order>(); }

    const auto& action_name() const noexcept
        requires(std::same_as<Kind, runir::kr::ps::ext::DoTag> || std::same_as<Kind, runir::kr::ps::ext::ActionTag>)
    { return self().template member<&urd::record_of<V>::action_name>(); }

    auto query_feature() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::ActionTag>
    { return self().template member<&urd::record_of<V>::query_feature>(); }

    auto callee() const noexcept
        requires std::same_as<Kind, runir::kr::ps::ext::CallTag>
    { return self().template member<&urd::record_of<V>::callee>(); }

    auto arguments() const noexcept
        requires(std::same_as<Kind, runir::kr::ps::ext::DoTag> || std::same_as<Kind, runir::kr::ps::ext::CallTag>)
    { return self().template member<&urd::record_of<V>::arguments>(); }
};
}  // namespace urd

#endif
