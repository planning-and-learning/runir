#include "runir/kr/ps/icp/formatter.hpp"

#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/dl/semantics/formatter.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/icp/execution_view.hpp"
#include "runir/kr/ps/icp/module_view.hpp"
#include "runir/kr/ps/icp/program_view.hpp"
#include "runir/kr/ps/icp/repository.hpp"
#include "runir/kr/ps/icp/rule_view.hpp"
#include "runir/kr/ps/icp/xcondition_view.hpp"
#include "runir/kr/ps/icp/xeffect_view.hpp"
#include "runir/kr/ps/memory_state_view.hpp"
#include "runir/kr/ps/module_symbol_view.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <tyr/planning/state_repository.hpp>
#include <urd/format.hpp>
#include <variant>
#include <yggdrasil/formatting/cista_formatters.hpp>

// A section of symbols is written on one line, (:name s1 s2 ...), so that an empty one stays closed: (:effects). A
// feature or a rule's binding names its feature by its symbol.

namespace rk = runir::kr;
namespace rp = runir::kr::ps;
namespace ri = runir::kr::ps::icp;

using ArgumentNames = ri::ArgumentNames<ri::RuleView<ri::CruleTag>>;

fmt::format_context::iterator fmt::formatter<ri::ConditionOperation>::format(ri::ConditionOperation value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value == ri::ConditionOperation::BELONGS ? "belongs" : "not-belongs"); }

fmt::format_context::iterator fmt::formatter<ri::EffectOperation>::format(ri::EffectOperation value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value == ri::EffectOperation::ENTER ? "enter" : "exit"); }

fmt::format_context::iterator fmt::formatter<ri::ArgumentPosition>::format(ri::ArgumentPosition value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value.value); }

fmt::format_context::iterator fmt::formatter<ri::ResetPair>::format(const ri::ResetPair& value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "({}, {})", value.before, value.after); }

template<typename X>
fmt::format_context::iterator
fmt::formatter<std::pair<urd::index_view<X, ri::Repository>, ArgumentNames>>::format(const std::pair<urd::index_view<X, ri::Repository>, ArgumentNames>& value,
                                                                                     fmt::format_context& context) const
{
    const auto& [predicate, names] = value;
    const auto& rule = names.rule;
    auto out = fmt::format_to(context.out(), "({}", predicate.get_operation());
    std::visit(
        [&]<typename Reference>(const Reference& reference)
        {
            if constexpr (std::same_as<Reference, ri::ArgumentPosition>)
            {
                const auto position = reference.value;
                if (rule && position < rule->get_argument_names().size())
                    out = fmt::format_to(out, " (:argument {})", rule->get_argument_names().at(position));
                else
                    out = fmt::format_to(out, " (:argument arg{})", position);
            }
            else
                out = fmt::format_to(out, " (:register (:concept {}))", reference.get_name());
        },
        predicate.get_object());
    return fmt::format_to(out, " (:concept {}))", std::visit([](auto concrete) { return concrete.get_symbol(); }, predicate.get_feature().get_variant()));
}

fmt::format_context::iterator
fmt::formatter<urd::index_view<ri::XCondition, ri::Repository>>::format(const urd::index_view<ri::XCondition, ri::Repository>& value,
                                                                        fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", std::pair { value, ArgumentNames {} }); }

fmt::format_context::iterator fmt::formatter<urd::index_view<ri::XEffect, ri::Repository>>::format(const urd::index_view<ri::XEffect, ri::Repository>& value,
                                                                                                   fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", std::pair { value, ArgumentNames {} }); }

template<ri::RuleKind Kind>
fmt::format_context::iterator fmt::formatter<ri::RuleView<Kind>>::format(const ri::RuleView<Kind>& value, fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(:{}", Kind::keyword);
    {
        const auto deeper = urd::scoped_indent();
        const auto write_conditions = [&]
        {
            out = fmt::format_to(out, "{}(:conditions", urd::new_line());
            for (const auto& condition : value.get_conditions())
                out = std::visit([&](auto concrete) { return fmt::format_to(out, " {}", concrete); }, condition);
            out = fmt::format_to(out, ")");
        };
        const auto write_effects = [&]
        {
            out = fmt::format_to(out, "{}(:effects", urd::new_line());
            for (const auto& effect : value.get_effects())
                out = std::visit([&](auto concrete) { return fmt::format_to(out, " {}", concrete); }, effect);
            out = fmt::format_to(out, ")");
        };

        if constexpr (std::same_as<Kind, ri::CruleTag>)
        {
            out = fmt::format_to(out, "{}(:action {:?})", urd::new_line(), value.get_action_name().view());
            out = fmt::format_to(out, "{}(:arguments", urd::new_line());
            for (const auto& name : value.get_argument_names())
                out = fmt::format_to(out, " {}", name);
            out = fmt::format_to(out, ")");
            write_conditions();
            out = fmt::format_to(out, "{}(:xconditions", urd::new_line());
            for (auto predicate : value.get_xconditions())
                out = fmt::format_to(out, " {}", std::pair { predicate, ArgumentNames { value } });
            out = fmt::format_to(out, ")");
            out = fmt::format_to(out, "{}(:xeffects", urd::new_line());
            for (auto predicate : value.get_xeffects())
                out = fmt::format_to(out, " {}", std::pair { predicate, ArgumentNames { value } });
            out = fmt::format_to(out, ")");
            write_effects();
        }
        else
        {
            using Category = ri::RuleCategoryFor<Kind>;
            write_conditions();
            out = fmt::format_to(out,
                                 "{}(:{} {})",
                                 urd::new_line(),
                                 Category::keyword,
                                 std::visit([](auto concrete) { return concrete.get_symbol(); }, value.get_feature().get_variant()));
            out = fmt::format_to(out, "{}(:register (:{} {}))", urd::new_line(), Category::keyword, value.get_slot().get_name());
            write_effects();
        }
    }
    return fmt::format_to(out, "{})", urd::new_line());
}

fmt::format_context::iterator fmt::formatter<ri::RuleVariantView>::format(const ri::RuleVariantView& value, fmt::format_context& context) const
{
    return std::visit([&](auto concrete) { return fmt::format_to(context.out(), "{}", concrete); }, value.get_variant());
}

fmt::format_context::iterator fmt::formatter<ri::ModuleView>::format(const ri::ModuleView& value, fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(:module");
    {
        const auto deeper = urd::scoped_indent();
        out = fmt::format_to(out, "{}(:symbol {})", urd::new_line(), value.get_symbol().get_name());
        out = fmt::format_to(out, "{}(:arguments)", urd::new_line());

        out = fmt::format_to(out, "{}(:registers", urd::new_line());
        for (auto register_ : value.get_registers<rk::dl::ConceptTag>())
            out = fmt::format_to(out, " (:{} {})", rk::dl::ConceptTag::keyword, register_.get_name());
        for (auto register_ : value.get_registers<rk::dl::RoleTag>())
            out = fmt::format_to(out, " (:{} {})", rk::dl::RoleTag::keyword, register_.get_name());
        out = fmt::format_to(out, ")");

        out = fmt::format_to(out, "{}(:entry {})", urd::new_line(), value.get_entry_memory_state().get_name());
        out = fmt::format_to(out, "{}(:memory", urd::new_line());
        for (auto state : value.get_memory_states())
            out = fmt::format_to(out, " {}", state.get_name());
        out = fmt::format_to(out, ")");

        out = fmt::format_to(out, "{}(:features", urd::new_line());
        {
            const auto features = urd::scoped_indent();
            const auto write_features = [&](auto range)
            {
                for (auto feature : range)
                    out = fmt::format_to(out, "{}{}", urd::new_line(), feature);
            };
            write_features(value.get_features<rk::dl::ConceptTag>());
            write_features(value.get_features<rk::dl::RoleTag>());
            write_features(value.get_features<rk::dl::BooleanTag>());
            write_features(value.get_features<rk::dl::NumericalTag>());
            write_features(value.get_query_features());
        }
        out = fmt::format_to(out, "{})", urd::new_line());

        out = fmt::format_to(out, "{}(:rules", urd::new_line());
        {
            const auto rules = urd::scoped_indent();
            for (const auto& transition : value.get_memory_transitions())
            {
                if (transition.empty())
                    continue;
                const auto first = transition.front();
                out = fmt::format_to(out, "{}(:rule", urd::new_line());
                {
                    const auto rule_sections = urd::scoped_indent();
                    out = fmt::format_to(out, "{}(:symbol {})", urd::new_line(), first.get_symbol());
                    out = fmt::format_to(out, "{}(:expression", urd::new_line());
                    {
                        const auto expression = urd::scoped_indent();
                        std::visit(
                            [&](auto first_rule)
                            {
                                out = fmt::format_to(out, "{}(:source-memory {})", urd::new_line(), first_rule.get_source().get_name());
                                if constexpr (rp::transition_rule<decltype(first_rule)>)
                                    out = fmt::format_to(out, "{}(:target-memory {})", urd::new_line(), first_rule.get_target().get_name());
                            },
                            first.get_variant());
                        for (auto rule : transition)
                            out = fmt::format_to(out, "{}{}", urd::new_line(), rule);
                    }
                    out = fmt::format_to(out, "{})", urd::new_line());
                }
                out = fmt::format_to(out, "{})", urd::new_line());
            }
        }
        out = fmt::format_to(out, "{})", urd::new_line());

        const auto symbol = [&](auto feature)
        { return std::visit([](auto concrete) { return concrete.get_symbol(); }, urd::resolve(feature, value.context()).get_variant()); };
        out = fmt::format_to(out, "{}(:reset-spo", urd::new_line());
        for (const auto& pair : value.get_reset_pairs())
            out = fmt::format_to(out, " (:prec {} {})", symbol(pair.before), symbol(pair.after));
        out = fmt::format_to(out, ")");
    }
    return fmt::format_to(out, "{})", urd::new_line());
}

fmt::format_context::iterator fmt::formatter<ri::ProgramView>::format(const ri::ProgramView& value, fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(:program");
    {
        const auto deeper = urd::scoped_indent();
        out = fmt::format_to(out, "{}(:entry {})", urd::new_line(), value.get_entry_mod().get_symbol().get_name());
        out = fmt::format_to(out, "{}{}", urd::new_line(), value.get_entry_mod());
    }
    return fmt::format_to(out, "{})", urd::new_line());
}

template<tyr::TaskKind Kind>
fmt::format_context::iterator
fmt::formatter<rp::ProgramProofVertexLabel<rk::IcpFamilyTag, Kind>>::format(const rp::ProgramProofVertexLabel<rk::IcpFamilyTag, Kind>& value,
                                                                            fmt::format_context& context) const
{
    return fmt::format_to(context.out(),
                          "state={} initial={} goal={} alive={} unsolvable={}",
                          value.program_state.get_state().index(),
                          value.is_initial,
                          value.is_goal,
                          value.is_alive,
                          value.is_unsolvable);
}

template struct fmt::formatter<std::pair<urd::index_view<ri::XCondition, ri::Repository>, ArgumentNames>>;
template struct fmt::formatter<std::pair<urd::index_view<ri::XEffect, ri::Repository>, ArgumentNames>>;
template struct fmt::formatter<ri::RuleView<ri::LoadTag<rk::dl::ConceptTag>>>;
template struct fmt::formatter<ri::RuleView<ri::LoadTag<rk::dl::RoleTag>>>;
template struct fmt::formatter<ri::RuleView<ri::CruleTag>>;
template struct fmt::formatter<rp::ProgramProofVertexLabel<rk::IcpFamilyTag, tyr::GroundTag>>;
template struct fmt::formatter<rp::ProgramProofVertexLabel<rk::IcpFamilyTag, tyr::LiftedTag>>;
