#include "runir/kr/ps/ext/formatter.hpp"

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/dl/semantics/formatter.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/order_term_view.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/memory_state_view.hpp"
#include "runir/kr/ps/module_symbol_view.hpp"
#include "runir/kr/ps/proof_status.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <urd/format.hpp>
#include <variant>
#include <yggdrasil/formatting/cista_formatters.hpp>

// A section of symbols is written on one line, (:name s1 s2 ...), so that an empty one stays closed: (:effects). A
// feature, a rule argument or an order term names its feature by its symbol.

namespace rk = runir::kr;
namespace rp = runir::kr::ps;
namespace re = runir::kr::ps::ext;

fmt::format_context::iterator fmt::formatter<re::OrderDirection>::format(re::OrderDirection value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value == re::OrderDirection::MIN ? "min" : "max"); }

template<re::RuleKind Kind>
fmt::format_context::iterator fmt::formatter<re::RuleView<Kind>>::format(const re::RuleView<Kind>& value, fmt::format_context& context) const
{
    const auto symbol = [](auto feature) { return std::visit([](auto concrete) { return concrete.symbol(); }, feature.variant()); };

    auto out = fmt::format_to(context.out(), "(:{}", Kind::keyword);
    {
        const auto deeper = urd::scoped_indent();
        out = fmt::format_to(out, "{}(:conditions", urd::new_line());
        for (const auto& condition : value.conditions())
            out = std::visit([&](auto concrete) { return fmt::format_to(out, " {}", concrete); }, condition);
        out = fmt::format_to(out, ")");

        const auto write_effects = [&](auto rule)
        {
            out = fmt::format_to(out, "{}(:effects", urd::new_line());
            for (const auto& effect : rule.effects())
                out = std::visit([&](auto concrete) { return fmt::format_to(out, " {}", concrete); }, effect);
            out = fmt::format_to(out, ")");
        };

        if constexpr (re::BindingRuleKind<Kind>)
        {
            using Category = re::RuleCategoryFor<Kind>;
            out = fmt::format_to(out, "{}(:{} {})", urd::new_line(), Category::keyword, symbol(value.feature()));
            out = fmt::format_to(out, "{}(:register", urd::new_line());
            {
                const auto slot = urd::scoped_indent();
                out = fmt::format_to(out, "{}(:{} {})", urd::new_line(), Category::keyword, value.slot().name());
            }
            out = fmt::format_to(out, "{})", urd::new_line());
            if (!value.effects().empty())
                write_effects(value);
            if constexpr (std::same_as<Kind, re::ChooseTag<Category>>)
            {
                if (!value.order().empty())
                {
                    out = fmt::format_to(out, "{}(:order", urd::new_line());
                    for (auto term : value.order())
                        out = fmt::format_to(out, " ({} {})", term.direction(), std::visit(symbol, term.feature()));
                    out = fmt::format_to(out, ")");
                }
            }
        }
        else if constexpr (std::same_as<Kind, re::SketchTag>)
        {
            write_effects(value);
        }
        else if constexpr (std::same_as<Kind, re::DoTag>)
        {
            out = fmt::format_to(out, "{}(:action {:?})", urd::new_line(), value.action_name().view());
            out = fmt::format_to(out, "{}(:arguments", urd::new_line());
            for (auto argument : value.arguments())
                out = fmt::format_to(out, " {}", symbol(argument));
            out = fmt::format_to(out, ")");
            write_effects(value);
        }
        else if constexpr (std::same_as<Kind, re::ActionTag>)
        {
            out = fmt::format_to(out, "{}(:action {:?})", urd::new_line(), value.action_name().view());
            out = fmt::format_to(out, "{}(:query {})", urd::new_line(), symbol(value.query_feature()));
            write_effects(value);
        }
        else if constexpr (std::same_as<Kind, re::CallTag>)
        {
            out = fmt::format_to(out, "{}(:callee {})", urd::new_line(), value.callee().name());
            out = fmt::format_to(out, "{}(:arguments", urd::new_line());
            for (const auto& argument : value.arguments())
                out = fmt::format_to(out, " {}", std::visit(symbol, argument));
            out = fmt::format_to(out, ")");
        }
    }
    return fmt::format_to(out, "{})", urd::new_line());
}

fmt::format_context::iterator fmt::formatter<re::RuleVariantView>::format(const re::RuleVariantView& value, fmt::format_context& context) const
{
    return std::visit([&](auto concrete) { return fmt::format_to(context.out(), "{}", concrete); }, value.variant());
}

fmt::format_context::iterator fmt::formatter<re::ModuleView>::format(const re::ModuleView& value, fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(:module");
    {
        const auto deeper = urd::scoped_indent();
        out = fmt::format_to(out, "{}(:symbol {})", urd::new_line(), value.symbol().name());

        const auto write_declarations = [&]<typename Category>(Category, auto declarations)
        {
            for (auto declaration : declarations)
                out = fmt::format_to(out, " (:{} {})", Category::keyword, declaration.name());
        };
        out = fmt::format_to(out, "{}(:arguments", urd::new_line());
        write_declarations(rk::dl::ConceptTag {}, value.arguments<rk::dl::ConceptTag>());
        write_declarations(rk::dl::RoleTag {}, value.arguments<rk::dl::RoleTag>());
        write_declarations(rk::dl::BooleanTag {}, value.arguments<rk::dl::BooleanTag>());
        write_declarations(rk::dl::NumericalTag {}, value.arguments<rk::dl::NumericalTag>());
        out = fmt::format_to(out, ")");
        out = fmt::format_to(out, "{}(:registers", urd::new_line());
        write_declarations(rk::dl::ConceptTag {}, value.registers<rk::dl::ConceptTag>());
        write_declarations(rk::dl::RoleTag {}, value.registers<rk::dl::RoleTag>());
        out = fmt::format_to(out, ")");

        out = fmt::format_to(out, "{}(:entry {})", urd::new_line(), value.entry_memory_state().name());
        out = fmt::format_to(out, "{}(:memory", urd::new_line());
        for (auto state : value.memory_states())
            out = fmt::format_to(out, " {}", state.name());
        out = fmt::format_to(out, ")");

        // The :datalog section as written, so that the module parses again.
        if (!value.datalog().empty())
            out = fmt::format_to(out, "{}(:datalog {})", urd::new_line(), value.datalog());

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
    }
    return fmt::format_to(out, "{})", urd::new_line());
}

fmt::format_context::iterator fmt::formatter<re::ProgramView>::format(const re::ProgramView& value, fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(:program");
    {
        const auto deeper = urd::scoped_indent();
        out = fmt::format_to(out, "{}(:entry {})", urd::new_line(), value.entry_mod().symbol().name());
        for (auto module_ : value.modules())
            out = fmt::format_to(out, "{}{}", urd::new_line(), module_);
    }
    return fmt::format_to(out, "{})", urd::new_line());
}

template<tyr::TaskKind Kind>
fmt::format_context::iterator fmt::formatter<urd::data<rp::ProgramProofVertexLabel<rk::ExtFamilyTag, Kind>>>::format(
    const urd::data<rp::ProgramProofVertexLabel<rk::ExtFamilyTag, Kind>>& value,
    fmt::format_context& context) const
{
    const auto state = value.program_state;
    return fmt::format_to(context.out(),
                          "state={} module={} initial={} goal={} alive={} unsolvable={}",
                          state.state().index(),
                          state.module_state().mod().symbol().name(),
                          value.is_initial,
                          value.is_goal,
                          value.is_alive,
                          value.is_unsolvable);
}

template<tyr::TaskKind Kind>
fmt::format_context::iterator fmt::formatter<re::ProgramProofResults<Kind>>::format(const re::ProgramProofResults<Kind>& value,
                                                                                    fmt::format_context& context) const
{
    return fmt::format_to(context.out(),
                          "ProgramProofResults(status={}, graph_vertices={}, graph_edges={}, deadend_states={}, open_states={}, cycle={}, "
                          "num_expanded={}, num_generated={}, num_backtracks={}, choice_depth={}, choice_width={})",
                          rp::to_string(value.status),
                          value.graph ? value.graph->num_vertices() : 0,
                          value.graph ? value.graph->num_edges() : 0,
                          value.deadend_states.size(),
                          value.open_states.size(),
                          value.cycle.size(),
                          value.statistics.num_expanded,
                          value.statistics.num_generated,
                          value.statistics.num_backtracks,
                          value.statistics.choice_depth,
                          value.statistics.choice_width);
}

template struct fmt::formatter<re::RuleView<re::LoadTag<rk::dl::ConceptTag>>>;
template struct fmt::formatter<re::RuleView<re::LoadTag<rk::dl::RoleTag>>>;
template struct fmt::formatter<re::RuleView<re::ChooseTag<rk::dl::ConceptTag>>>;
template struct fmt::formatter<re::RuleView<re::ChooseTag<rk::dl::RoleTag>>>;
template struct fmt::formatter<re::RuleView<re::SketchTag>>;
template struct fmt::formatter<re::RuleView<re::DoTag>>;
template struct fmt::formatter<re::RuleView<re::CallTag>>;
template struct fmt::formatter<re::RuleView<re::ActionTag>>;
template struct fmt::formatter<re::RuleView<re::BacktrackTag>>;
template struct fmt::formatter<urd::data<rp::ProgramProofVertexLabel<rk::ExtFamilyTag, tyr::GroundTag>>>;
template struct fmt::formatter<urd::data<rp::ProgramProofVertexLabel<rk::ExtFamilyTag, tyr::LiftedTag>>>;
template struct fmt::formatter<re::ProgramProofResults<tyr::GroundTag>>;
template struct fmt::formatter<re::ProgramProofResults<tyr::LiftedTag>>;
