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
#include <tyr/planning/states/state_repository.hpp>
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
{ return fmt::format_to(context.out(), "{}", ri::to_string(value)); }

fmt::format_context::iterator fmt::formatter<ri::EffectOperation>::format(ri::EffectOperation value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", ri::to_string(value)); }

fmt::format_context::iterator fmt::formatter<ri::ArgumentPosition>::format(ri::ArgumentPosition value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value.value); }

fmt::format_context::iterator fmt::formatter<ri::ResetPairView>::format(const ri::ResetPairView& value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "({}, {})", value.before(), value.after()); }

template<typename X>
fmt::format_context::iterator
fmt::formatter<std::pair<urd::index_view<X, ri::Repository>, ArgumentNames>>::format(const std::pair<urd::index_view<X, ri::Repository>, ArgumentNames>& value,
                                                                                     fmt::format_context& context) const
{
    const auto& [predicate, names] = value;
    const auto& rule = names.rule;
    auto out = fmt::format_to(context.out(), "({}", predicate.operation());
    std::visit(
        [&]<typename Reference>(const Reference& reference)
        {
            if constexpr (std::same_as<Reference, ri::ArgumentPosition>)
            {
                const auto position = reference.value;
                if (rule && position < rule->argument_names().size())
                    out = fmt::format_to(out, " (:argument {})", rule->argument_names().at(position));
                else
                    out = fmt::format_to(out, " (:argument arg{})", position);
            }
            else
                out = fmt::format_to(out, " (:register (:concept {}))", reference.name());
        },
        predicate.object());
    return fmt::format_to(out, " (:concept {}))", std::visit([](auto concrete) { return concrete.symbol(); }, predicate.feature().variant()));
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
            for (const auto& condition : value.conditions())
                out = std::visit([&](auto concrete) { return fmt::format_to(out, " {}", concrete); }, condition);
            out = fmt::format_to(out, ")");
        };
        const auto write_effects = [&]
        {
            out = fmt::format_to(out, "{}(:effects", urd::new_line());
            for (const auto& effect : value.effects())
                out = std::visit([&](auto concrete) { return fmt::format_to(out, " {}", concrete); }, effect);
            out = fmt::format_to(out, ")");
        };

        if constexpr (std::same_as<Kind, ri::CruleTag>)
        {
            out = fmt::format_to(out, "{}(:action {:?})", urd::new_line(), value.action_name().view());
            out = fmt::format_to(out, "{}(:arguments", urd::new_line());
            for (const auto& name : value.argument_names())
                out = fmt::format_to(out, " {}", name);
            out = fmt::format_to(out, ")");
            write_conditions();
            out = fmt::format_to(out, "{}(:xconditions", urd::new_line());
            for (auto predicate : value.xconditions())
                out = fmt::format_to(out, " {}", std::pair { predicate, ArgumentNames { value } });
            out = fmt::format_to(out, ")");
            out = fmt::format_to(out, "{}(:xeffects", urd::new_line());
            for (auto predicate : value.xeffects())
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
                                 std::visit([](auto concrete) { return concrete.symbol(); }, value.feature().variant()));
            out = fmt::format_to(out, "{}(:register (:{} {}))", urd::new_line(), Category::keyword, value.slot().name());
            write_effects();
        }
    }
    return fmt::format_to(out, "{})", urd::new_line());
}

fmt::format_context::iterator fmt::formatter<ri::RuleVariantView>::format(const ri::RuleVariantView& value, fmt::format_context& context) const
{
    return std::visit([&](auto concrete) { return fmt::format_to(context.out(), "{}", concrete); }, value.variant());
}

fmt::format_context::iterator fmt::formatter<ri::ModuleView>::format(const ri::ModuleView& value, fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(:module");
    {
        const auto deeper = urd::scoped_indent();
        out = fmt::format_to(out, "{}(:symbol {})", urd::new_line(), value.symbol().name());
        out = fmt::format_to(out, "{}(:arguments)", urd::new_line());

        out = fmt::format_to(out, "{}(:registers", urd::new_line());
        for (auto register_ : value.registers<rk::dl::ConceptTag>())
            out = fmt::format_to(out, " (:{} {})", rk::dl::ConceptTag::keyword, register_.name());
        for (auto register_ : value.registers<rk::dl::RoleTag>())
            out = fmt::format_to(out, " (:{} {})", rk::dl::RoleTag::keyword, register_.name());
        out = fmt::format_to(out, ")");

        out = fmt::format_to(out, "{}(:entry {})", urd::new_line(), value.entry_memory_state().name());
        out = fmt::format_to(out, "{}(:memory", urd::new_line());
        for (auto state : value.memory_states())
            out = fmt::format_to(out, " {}", state.name());
        out = fmt::format_to(out, ")");

        out = fmt::format_to(out, "{}(:features", urd::new_line());
        {
            const auto features = urd::scoped_indent();
            const auto write_features = [&](auto range)
            {
                for (auto feature : range)
                    out = fmt::format_to(out, "{}{}", urd::new_line(), feature);
            };
            write_features(value.features<rk::dl::ConceptTag>());
            write_features(value.features<rk::dl::RoleTag>());
            write_features(value.features<rk::dl::BooleanTag>());
            write_features(value.features<rk::dl::NumericalTag>());
            write_features(value.query_features());
        }
        out = fmt::format_to(out, "{})", urd::new_line());

        out = fmt::format_to(out, "{}(:rules", urd::new_line());
        {
            const auto rules = urd::scoped_indent();
            for (const auto& transition : value.memory_transitions())
            {
                if (transition.empty())
                    continue;
                const auto first = transition.front();
                out = fmt::format_to(out, "{}(:rule", urd::new_line());
                {
                    const auto rule_sections = urd::scoped_indent();
                    out = fmt::format_to(out, "{}(:symbol {})", urd::new_line(), first.symbol());
                    out = fmt::format_to(out, "{}(:expression", urd::new_line());
                    {
                        const auto expression = urd::scoped_indent();
                        std::visit(
                            [&](auto first_rule)
                            {
                                out = fmt::format_to(out, "{}(:source-memory {})", urd::new_line(), first_rule.source().name());
                                if constexpr (rp::transition_rule<decltype(first_rule)>)
                                    out = fmt::format_to(out, "{}(:target-memory {})", urd::new_line(), first_rule.target().name());
                            },
                            first.variant());
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
        { return std::visit([](auto concrete) { return concrete.symbol(); }, urd::resolve(feature, value.context()).variant()); };
        out = fmt::format_to(out, "{}(:reset-spo", urd::new_line());
        for (const auto& pair : value.reset_pairs())
            out = fmt::format_to(out, " (:prec {} {})", symbol(pair.before().index()), symbol(pair.after().index()));
        out = fmt::format_to(out, ")");
    }
    return fmt::format_to(out, "{})", urd::new_line());
}

fmt::format_context::iterator fmt::formatter<ri::ProgramView>::format(const ri::ProgramView& value, fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(:program");
    {
        const auto deeper = urd::scoped_indent();
        out = fmt::format_to(out, "{}(:entry {})", urd::new_line(), value.entry_mod().symbol().name());
        out = fmt::format_to(out, "{}{}", urd::new_line(), value.entry_mod());
    }
    return fmt::format_to(out, "{})", urd::new_line());
}

template<tyr::TaskKind Kind>
fmt::format_context::iterator fmt::formatter<urd::data<rp::ProgramProofVertexLabel<rk::IcpFamilyTag, Kind>>>::format(
    const urd::data<rp::ProgramProofVertexLabel<rk::IcpFamilyTag, Kind>>& value,
    fmt::format_context& context) const
{
    return fmt::format_to(context.out(),
                          "state={} initial={} goal={} alive={} unsolvable={}",
                          value.program_state.state().pack().index(),
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
template struct fmt::formatter<urd::data<rp::ProgramProofVertexLabel<rk::IcpFamilyTag, tyr::GroundTag>>>;
template struct fmt::formatter<urd::data<rp::ProgramProofVertexLabel<rk::IcpFamilyTag, tyr::LiftedTag>>>;
