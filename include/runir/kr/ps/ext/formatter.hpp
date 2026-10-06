#ifndef RUNIR_KR_PS_EXT_FORMATTER_HPP_
#define RUNIR_KR_PS_EXT_FORMATTER_HPP_

#include "runir/kr/ps/ext/dl/formatter.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/kr/ps/ext/views.hpp"
#include "runir/kr/ps/formatter.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <tyr/formalism/planning/formatter.hpp>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps::ext
{

template<typename Values, typename Append>
void append_inline_section(std::ostream& os, std::string_view name, Values values, Append append)
{
    os << ygg::print_indent << "(:" << name;
    for (auto value : values)
    {
        os << ' ';
        append(os, value);
    }
    os << ")\n";
}

template<typename Conditions>
void append_conditions(std::ostream& os, Conditions conditions)
{
    append_inline_section(os, "conditions", conditions, [](std::ostream& output, auto value) { condition(value, std::ostream_iterator<char>(output)); });
}

template<typename Effects>
void append_effects(std::ostream& os, Effects effects)
{
    append_inline_section(os, "effects", effects, [](std::ostream& output, auto value) { effect(value, std::ostream_iterator<char>(output)); });
}

template<runir::kr::ps::ext::RuleKind Kind, typename C>
void append_rule_body(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C> view)
{
    append_conditions(os, view.get_conditions());
    if constexpr (runir::kr::ps::ext::BindingRuleKind<Kind>)
    {
        using Category = RuleCategoryFor<Kind>;
        os << ygg::print_indent << "(:" << Category::name << ' ' << view.get_feature().get_symbol() << ")\n";
        os << ygg::print_indent << "(:register\n";
        {
            ygg::IndentScope scope(os);
            os << ygg::print_indent << "(:" << Category::name << ' ' << view.get_register().get_name() << ")\n";
        }
        os << ygg::print_indent << ")\n";
        if (!view.get_effects().empty())
            append_effects(os, view.get_effects());
        if constexpr (std::same_as<Kind, ChooseTag<Category>>)
            if (!view.get_order().empty())
                append_inline_section(os,
                                      "order",
                                      view.get_order(),
                                      [](std::ostream& output, auto term)
                                      {
                                          output << '(' << (term.get_direction() == OrderDirection::MIN ? "min" : "max") << ' ';
                                          ygg::visit([&](auto feature) { output << feature.get_symbol(); }, term.get_feature());
                                          output << ')';
                                      });
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::SketchTag>)
    {
        append_effects(os, view.get_effects());
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::DoTag>)
    {
        os << ygg::print_indent << "(:action " << fmt::format("{:?}", view.get_action_name().view()) << ")\n";
        append_inline_section(os, "arguments", view.get_action_arguments(), [](std::ostream& output, auto argument) { output << argument.get_symbol(); });
        append_effects(os, view.get_effects());
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::ActionTag>)
    {
        os << ygg::print_indent << "(:action " << fmt::format("{:?}", view.get_action_name().view()) << ")\n";
        os << ygg::print_indent << "(:query " << view.get_query_feature().get_symbol() << ")\n";
        append_effects(os, view.get_effects());
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::CallTag>)
    {
        os << ygg::print_indent << "(:callee " << view.get_callee().get_name() << ")\n";
        os << ygg::print_indent << "(:arguments";
        view.for_each_call_argument([&](auto argument) { os << ' ' << argument.get_symbol(); });
        os << ")\n";
    }
}

template<runir::kr::ps::ext::RuleKind Kind, typename C>
void append_rule(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C> view)
{
    os << ygg::print_indent << "(:" << Kind::keyword << "\n";
    {
        ygg::IndentScope scope(os);
        append_rule_body(os, view);
    }
    os << ygg::print_indent << ')';
}

template<typename C>
void append_rule(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>, C> view)
{
    ygg::visit([&](auto rule) { append_rule(os, rule); }, view.get_variant());
}

template<typename View>
std::string rule(View view)
{
    auto os = std::ostringstream {};
    append_rule(os, view);
    return os.str();
}

template<runir::kr::dl::CategoryTag Category, typename Arguments>
void append_declarations(std::ostream& os, Arguments arguments)
{
    for (auto argument : arguments)
        os << " (:" << Category::name << ' ' << argument.get_name() << ')';
}

template<typename C>
void append_module(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ext::Module>, C> view)
{
    os << ygg::print_indent << "(:module\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << "(:symbol " << view.get_name() << ")\n";

        os << ygg::print_indent << "(:arguments";
        append_declarations<runir::kr::dl::ConceptTag>(os, view.template get_arguments<runir::kr::dl::ConceptTag>());
        append_declarations<runir::kr::dl::RoleTag>(os, view.template get_arguments<runir::kr::dl::RoleTag>());
        append_declarations<runir::kr::dl::BooleanTag>(os, view.template get_arguments<runir::kr::dl::BooleanTag>());
        append_declarations<runir::kr::dl::NumericalTag>(os, view.template get_arguments<runir::kr::dl::NumericalTag>());
        os << ")\n";

        os << ygg::print_indent << "(:registers";
        append_declarations<runir::kr::dl::ConceptTag>(os, view.template get_registers<runir::kr::dl::ConceptTag>());
        append_declarations<runir::kr::dl::RoleTag>(os, view.template get_registers<runir::kr::dl::RoleTag>());
        os << ")\n";

        os << ygg::print_indent << "(:entry " << view.get_entry_memory_state().get_name() << ")\n";
        os << ygg::print_indent << "(:memory";
        for (auto state : view.get_memory_states())
            os << ' ' << state.get_name();
        os << ")\n";

        os << ygg::print_indent << "(:features\n";
        {
            ygg::IndentScope feature_scope(os);
            const auto append_features = [&](auto features)
            {
                for (auto feature : features)
                {
                    append_feature(os, feature);
                    os << '\n';
                }
            };
            append_features(view.template get_features<runir::kr::dl::ConceptTag>());
            append_features(view.template get_features<runir::kr::dl::RoleTag>());
            append_features(view.template get_features<runir::kr::ps::dl::BooleanFeature>());
            append_features(view.template get_features<runir::kr::ps::dl::NumericalFeature>());
            append_features(view.get_query_features());
        }
        os << ygg::print_indent << ")\n";

        os << ygg::print_indent << "(:rules\n";
        {
            ygg::IndentScope rules_scope(os);
            for (const auto& transition : view.get_memory_transitions())
            {
                if (transition.empty())
                    continue;
                const auto first = transition.front();
                os << ygg::print_indent << "(:rule\n";
                {
                    ygg::IndentScope rule_scope(os);
                    os << ygg::print_indent << "(:symbol " << first.get_symbol() << ")\n";
                    os << ygg::print_indent << "(:expression\n";
                    {
                        ygg::IndentScope expression_scope(os);
                        ygg::visit(
                            [&](auto first_rule)
                            {
                                os << ygg::print_indent << "(:source-memory " << first_rule.get_source().get_name() << ")\n";
                                os << ygg::print_indent << "(:target-memory " << first_rule.get_target().get_name() << ")\n";
                            },
                            first.get_variant());
                        for (auto item : transition)
                        {
                            append_rule(os, item);
                            os << '\n';
                        }
                    }
                    os << ygg::print_indent << ")\n";
                }
                os << ygg::print_indent << ")\n";
            }
        }
        os << ygg::print_indent << ")\n";
    }
    os << ygg::print_indent << ')';
}

template<typename C>
std::string module(ygg::View<ygg::Index<runir::kr::ps::ext::Module>, C> view)
{
    auto os = std::ostringstream {};
    append_module(os, view);
    return os.str();
}

template<typename C>
void append_program(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ext::Program>, C> view)
{
    os << ygg::print_indent << "(:program\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << "(:entry " << view.get_entry_module().get_name() << ")\n";
        for (auto module_ : view.get_modules())
        {
            append_module(os, module_);
            os << '\n';
        }
    }
    os << ygg::print_indent << ')';
}

template<typename C>
std::string program(ygg::View<ygg::Index<runir::kr::ps::ext::Program>, C> view)
{
    auto os = std::ostringstream {};
    append_program(os, view);
    return os.str();
}

}  // namespace runir::kr::ps::ext

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::MemoryState>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return fmt::format_to(context.out(), "{}", view.get_name()); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::ModuleSymbol>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return fmt::format_to(context.out(), "{}", view.get_name()); }
};

template<runir::kr::ps::ext::RuleKind Kind, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::rule(view), context); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::rule(view), context); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::Module>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::module(view), context); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::Program>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::program(view), context); }
};

template<tyr::TaskKind Kind, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::ModuleState<Kind>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& value, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "ModuleState(state={}, module={}, memory_state={}, registers={}, arguments={})",
                              value.get_state().get_index(),
                              value.get_module().get_name(),
                              value.get_memory_state(),
                              value.get_registers(),
                              value.get_arguments());
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::CallStack>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& value, format_context& context) const -> format_context::iterator
    {
        return fmt::format_to(context.out(),
                              "CallStack(module={}, return_memory_state={}, registers={}, arguments={}, caller={})",
                              value.get_module().get_name(),
                              value.get_return_memory_state(),
                              value.get_registers(),
                              value.get_arguments(),
                              value.get_caller());
    }
};

template<tyr::TaskKind Kind, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::ProgramState<Kind>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& value, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "ProgramState(program={}, module_state={}, call_stack={})",
                              value.get_program().get_index(),
                              value.get_module_state(),
                              value.get_call_stack());
    }
};

template<tyr::TaskKind Kind>
struct fmt::formatter<runir::kr::ps::ext::ProgramProofVertexLabel<Kind>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& label, format_context& context) const
    {
        const auto state = label.program_state;
        return fmt::format_to(context.out(),
                              "state={} module={} initial={} goal={} alive={} unsolvable={}",
                              state.get_state().get_index(),
                              state.get_module_state().get_module().get_name(),
                              label.is_initial,
                              label.is_goal,
                              label.is_alive,
                              label.is_unsolvable);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::ext::ProgramProofEdgeLabel>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& label, format_context& context) const
    {
        if (label.rule)
            return fmt::format_to(context.out(), "rule={}", label.rule->get_symbol());
        return fmt::format_to(context.out(), "rule=<none>");
    }
};

template<tyr::TaskKind Kind>
struct fmt::formatter<runir::kr::ps::ext::ProgramProofResults<Kind>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& result, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "ProgramProofResults(status={}, graph_vertices={}, graph_edges={}, deadend_states={}, open_states={}, cycle={}, "
                              "num_expanded={}, num_generated={}, choice_depth={}, choice_width={})",
                              runir::kr::ps::ext::to_string(result.status),
                              result.graph ? result.graph->get_num_vertices() : 0,
                              result.graph ? result.graph->get_num_edges() : 0,
                              result.deadend_states.size(),
                              result.open_states.size(),
                              result.cycle.size(),
                              result.statistics.num_expanded,
                              result.statistics.num_generated,
                              result.statistics.choice_depth,
                              result.statistics.choice_width);
    }
};

#endif
