#ifndef RUNIR_KR_PS_EXT_FORMATTER_HPP_
#define RUNIR_KR_PS_EXT_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/kr/dl/semantics/formatter.hpp"
#include "runir/kr/ps/ext/dl/formatter.hpp"
#include "runir/kr/ps/ext/module_program_executor_data.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/kr/ps/ext/views.hpp"

#include <concepts>
#include <fmt/format.h>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps::ext::format
{

template<typename FeatureTag>
constexpr std::string_view feature_type()
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        return runir::kr::dl::ConceptTag::name;
    else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
        return runir::kr::dl::RoleTag::name;
    else
        return FeatureTag::keyword;
}

template<typename FeatureTag, typename C>
void append_feature(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view)
{
    os << ygg::print_indent << "(:" << feature_type<FeatureTag>() << "\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << "(:symbol " << std::string(view.get_symbol()) << ")\n";
        os << ygg::print_indent << "(:expression ";
        fmt::format_to(std::ostream_iterator<char>(os), "{}", view.get_expression());
        os << ")\n";
    }
    os << ygg::print_indent << ')';
}

template<typename FeatureTag, typename C>
void append_feature(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> view)
{
    ygg::visit([&](auto concrete) { append_feature(os, concrete); }, view.get_variant());
}

template<typename View>
std::string feature(View view)
{
    auto os = std::ostringstream {};
    append_feature(os, view);
    return os.str();
}

template<typename Out, typename FeatureTag, typename ObservationTag, typename C>
Out condition(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view, Out out)
{
    return fmt::format_to(out, "({} {})", ObservationTag::keyword, std::string(view.get_feature().get_symbol()));
}

template<typename Out, typename LanguageTag, typename C>
Out condition(ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, LanguageTag>>, C> view, Out out)
{
    return ygg::visit([&](auto concrete) { return condition(concrete, out); }, view.get_variant());
}

template<typename Out, typename C>
Out condition(ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C> view, Out out)
{
    return ygg::visit([&](auto concrete) { return condition(concrete, out); }, view.get_variant());
}

template<typename Out, typename FeatureTag, typename ObservationTag, typename C>
Out effect(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view, Out out)
{
    return fmt::format_to(out, "({} {})", ObservationTag::keyword, std::string(view.get_feature().get_symbol()));
}

template<typename Out, typename LanguageTag, typename C>
Out effect(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, LanguageTag>>, C> view, Out out)
{
    return ygg::visit([&](auto concrete) { return effect(concrete, out); }, view.get_variant());
}

template<typename Out, typename C>
Out effect(ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>, C> view, Out out)
{
    return ygg::visit([&](auto concrete) { return effect(concrete, out); }, view.get_variant());
}

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
    if constexpr (std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::ConceptTag>>
                  || std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::RoleTag>>)
    {
        using Category =
            std::conditional_t<std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::ConceptTag>>, runir::kr::dl::ConceptTag, runir::kr::dl::RoleTag>;
        os << ygg::print_indent << "(:" << Category::name << ' ' << std::string(view.get_feature().get_symbol()) << ")\n";
        os << ygg::print_indent << "(:register\n";
        {
            ygg::IndentScope scope(os);
            os << ygg::print_indent << "(:" << Category::name << ' ' << std::string(view.get_register().get_name()) << ")\n";
        }
        os << ygg::print_indent << ")\n";
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::SketchTag>)
    {
        append_effects(os, view.get_effects());
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::DoTag>)
    {
        os << ygg::print_indent << "(:action " << fmt::format("{:?}", std::string(view.get_action_name())) << ")\n";
        append_inline_section(os,
                              "arguments",
                              view.get_action_arguments(),
                              [](std::ostream& output, auto argument) { output << std::string(argument.get_symbol()); });
        append_effects(os, view.get_effects());
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::CallTag>)
    {
        os << ygg::print_indent << "(:callee " << std::string(view.get_callee().get_name()) << ")\n";
        os << ygg::print_indent << "(:arguments";
        view.for_each_call_argument([&](auto argument) { os << ' ' << std::string(argument.get_symbol()); });
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
void append_rule(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ext::RuleVariant>, C> view)
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
        os << " (:" << Category::name << ' ' << std::string(argument.get_name()) << ')';
}

template<typename C>
void append_module(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ext::Module>, C> view)
{
    os << ygg::print_indent << "(:module\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << "(:symbol " << std::string(view.get_name()) << ")\n";

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

        os << ygg::print_indent << "(:entry " << std::string(view.get_entry_memory_state().get_name()) << ")\n";
        os << ygg::print_indent << "(:memory";
        for (auto state : view.get_memory_states())
            os << ' ' << std::string(state.get_name());
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
                    os << ygg::print_indent << "(:symbol " << std::string(first.get_symbol()) << ")\n";
                    os << ygg::print_indent << "(:expression\n";
                    {
                        ygg::IndentScope expression_scope(os);
                        ygg::visit(
                            [&](auto first_rule)
                            {
                                os << ygg::print_indent << "(:source-memory " << std::string(first_rule.get_source().get_name()) << ")\n";
                                os << ygg::print_indent << "(:target-memory " << std::string(first_rule.get_target().get_name()) << ")\n";
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
void append_module_program(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ext::ModuleProgram>, C> view)
{
    os << ygg::print_indent << "(:program\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << "(:entry " << std::string(view.get_entry_module().get_name()) << ")\n";
        for (auto module : view.get_modules())
        {
            append_module(os, module);
            os << '\n';
        }
    }
    os << ygg::print_indent << ')';
}

template<typename C>
std::string module_program(ygg::View<ygg::Index<runir::kr::ps::ext::ModuleProgram>, C> view)
{
    auto os = std::ostringstream {};
    append_module_program(os, view);
    return os.str();
}

inline std::string_view module_program_proof_status(runir::kr::ps::ext::ModuleProgramProofStatus status)
{
    switch (status)
    {
        case runir::kr::ps::ext::ModuleProgramProofStatus::SUCCESS:
            return "success";
        case runir::kr::ps::ext::ModuleProgramProofStatus::FAILURE:
            return "failure";
        case runir::kr::ps::ext::ModuleProgramProofStatus::OUT_OF_TIME:
            return "out_of_time";
        case runir::kr::ps::ext::ModuleProgramProofStatus::OUT_OF_STATES:
            return "out_of_states";
    }
    return "unknown";
}

}  // namespace runir::kr::ps::ext::format

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::MemoryState>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return fmt::format_to(context.out(), "{}", std::string(view.get_name())); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::ModuleSymbol>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return fmt::format_to(context.out(), "{}", std::string(view.get_name())); }
};

template<typename FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::feature(view), context);
    }
};

template<typename FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C>> :
    fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::feature(view), context);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return runir::kr::ps::ext::format::condition(view, context.out()); }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, LanguageTag>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return runir::kr::ps::ext::format::condition(view, context.out()); }
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, LanguageTag, FeatureTag, ObservationTag>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return runir::kr::ps::ext::format::condition(view, context.out()); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return runir::kr::ps::ext::format::effect(view, context.out()); }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, LanguageTag>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return runir::kr::ps::ext::format::effect(view, context.out()); }
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, LanguageTag, FeatureTag, ObservationTag>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return runir::kr::ps::ext::format::effect(view, context.out()); }
};

template<runir::kr::ps::ext::RuleKind Kind, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::rule(view), context); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::RuleVariant>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::rule(view), context); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::Module>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::module(view), context);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::ModuleProgram>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::module_program(view), context);
    }
};

template<tyr::planning::TaskKind Kind>
struct fmt::formatter<runir::kr::ps::ext::ModuleProgramProofVertexLabel<Kind>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& label, format_context& context) const
    {
        const auto state = label.execution_state;
        return fmt::format_to(context.out(),
                              "state={} module={} initial={} goal={} alive={} unsolvable={}",
                              ygg::uint_t(state.get_state().get_index()),
                              std::string(state.get_call_stack().get_module().get_name()),
                              label.is_initial,
                              label.is_goal,
                              label.is_alive,
                              label.is_unsolvable);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::ext::ModuleProgramProofEdgeLabel>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& label, format_context& context) const
    {
        if (label.rule)
            return fmt::format_to(context.out(), "rule={}", std::string(label.rule->get_symbol()));
        return fmt::format_to(context.out(), "rule=<none>");
    }
};

template<tyr::planning::TaskKind Kind>
struct fmt::formatter<runir::kr::ps::ext::ModuleProgramProofResults<Kind>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& result, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "ModuleProgramProofResults(status={}, graph_vertices={}, graph_edges={}, deadend_states={}, open_states={}, cycle={})",
                              runir::kr::ps::ext::format::module_program_proof_status(result.status),
                              result.graph ? result.graph->get_num_vertices() : 0,
                              result.graph ? result.graph->get_num_edges() : 0,
                              result.deadend_states.size(),
                              result.open_states.size(),
                              result.cycle.size());
    }
};

#endif
