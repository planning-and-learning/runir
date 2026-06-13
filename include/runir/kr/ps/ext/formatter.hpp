#ifndef RUNIR_KR_PS_EXT_FORMATTER_HPP_
#define RUNIR_KR_PS_EXT_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/formatter.hpp"
#include "runir/kr/ps/ext/dl/formatter.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/kr/ps/ext/views.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps::ext::format
{

inline std::string symbol_section(std::string_view value) { return fmt::format("(:symbol {})", value); }

template<typename FeatureTag>
struct NamedFeature
{
    ygg::Index<runir::kr::ps::ext::Feature<FeatureTag>> index;
    std::string name;
};

struct FeatureNames
{
    std::vector<NamedFeature<runir::kr::dl::ConceptTag>> concepts;
    std::vector<NamedFeature<runir::kr::ps::dl::BooleanFeature>> booleans;
    std::vector<NamedFeature<runir::kr::ps::dl::NumericalFeature>> numericals;
};

template<typename FeatureTag>
auto& named_features(FeatureNames& names)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        return names.concepts;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
        return names.booleans;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
        return names.numericals;
}

template<typename FeatureTag>
std::string feature_prefix()
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        return "c";
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
        return "b";
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
        return "n";
}

template<typename FeatureTag, typename C>
std::string feature_symbol(ygg::View<ygg::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C> feature)
{
    return ygg::visit([](auto concrete_feature) { return std::string(concrete_feature.get_symbol()); }, feature.get_variant());
}

template<typename FeatureTag, typename C>
std::string get_or_create_name(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C> feature)
{
    auto& entries = named_features<FeatureTag>(names);
    for (const auto& entry : entries)
        if (entry.index == feature.get_index())
            return entry.name;

    auto name = feature_symbol(feature);
    entries.push_back(NamedFeature<FeatureTag> { feature.get_index(), name });
    return name;
}

template<typename FeatureTag, typename ObservationTag, typename C>
void collect_feature(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    get_or_create_name(names, view.get_feature());
}

template<typename FeatureTag, typename ObservationTag, typename C>
void collect_feature(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    get_or_create_name(names, view.get_feature());
}

template<typename C>
void collect_features(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::ConditionVariant>, C> view)
{
    ygg::visit([&](auto concrete_variant)
               { ygg::visit([&](auto concrete_condition) { collect_feature(names, concrete_condition); }, concrete_variant.get_variant()); },
               view.get_variant());
}

template<typename C>
void collect_features(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::EffectVariant>, C> view)
{
    ygg::visit([&](auto concrete_variant)
               { ygg::visit([&](auto concrete_effect) { collect_feature(names, concrete_effect); }, concrete_variant.get_variant()); },
               view.get_variant());
}

template<typename Kind, typename C>
void collect_features(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C> view)
{
    for (auto condition : view.get_conditions())
        collect_features(names, condition);
    if constexpr (requires { view.get_effects(); })
        for (auto effect : view.get_effects())
            collect_features(names, effect);
    if constexpr (std::same_as<Kind, runir::kr::ps::ext::DoTag>)
        for (auto argument : view.get_action_arguments())
            get_or_create_name(names, argument);
}

template<typename C>
void collect_features(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::RuleVariant>, C> view)
{
    ygg::visit([&](auto rule) { collect_features(names, rule); }, view.get_variant());
}

template<typename C>
FeatureNames collect_features(ygg::View<ygg::Index<runir::kr::ps::ext::Module>, C> view)
{
    auto names = FeatureNames {};
    for (const auto& transition : view.get_memory_transitions())
        for (auto rule : transition)
            collect_features(names, rule);
    return names;
}

template<typename FeatureTag>
std::string feature_type()
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        return runir::kr::dl::ConceptTag::name;
    else
        return FeatureTag::keyword;
}

inline void append_value(std::ostream& os, std::string_view value)
{
    auto stream = std::istringstream(std::string(value));
    auto line = std::string {};
    auto first = true;
    while (std::getline(stream, line))
    {
        if (!first)
            os << '\n';
        os << ygg::print_indent << line;
        first = false;
    }
}

inline void append_list_section(std::ostream& os, std::string_view name, const std::vector<std::string>& values)
{
    if (values.empty())
    {
        os << ygg::print_indent << fmt::format("(:{})", name) << "\n";
        return;
    }

    os << ygg::print_indent << fmt::format("(:{}\n", name);
    {
        ygg::IndentScope scope(os);
        for (const auto& value : values)
        {
            append_value(os, value);
            os << "\n";
        }
    }
    os << ygg::print_indent << ")\n";
}

inline void append_value_section(std::ostream& os, std::string_view name, std::string_view value)
{
    os << ygg::print_indent << fmt::format("(:{}\n", name);
    {
        ygg::IndentScope scope(os);
        append_value(os, value);
        os << "\n";
    }
    os << ygg::print_indent << ")\n";
}

template<typename FeatureTag, typename C>
void append_feature(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C> view, std::string_view)
{
    os << ygg::print_indent << fmt::format("(:{}\n", feature_type<FeatureTag>());
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << symbol_section(std::string(view.get_symbol().str())) << "\n";
        os << ygg::print_indent << fmt::format("(:description {})", fmt::format("{:?}", std::string(view.get_description().str()))) << "\n";
        append_value_section(os, "expression", runir::kr::ps::ext::dl::format::expression(view.get_feature()));
    }
    os << ygg::print_indent << ")";
}

template<typename FeatureTag, typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C> view, std::string_view name)
{
    auto os = std::ostringstream {};
    append_feature(os, view, name);
    return os.str();
}

template<typename FeatureTag, typename C>
void append_feature(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C> view, std::string_view name)
{
    ygg::visit([&](auto concrete_feature) { append_feature(os, concrete_feature, name); }, view.get_variant());
}

template<typename FeatureTag, typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C> view, std::string_view name)
{
    auto os = std::ostringstream {};
    append_feature(os, view, name);
    return os.str();
}

template<typename FeatureTag, typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C> view)
{
    return feature(view, fmt::format("{}_{}", feature_prefix<FeatureTag>(), view.get_index().get_value()));
}

template<typename C>
void append_features(std::ostream& os, const C& context, const FeatureNames& names)
{
    for (const auto& entry : names.concepts)
    {
        append_feature(os, ygg::make_view(entry.index, context), entry.name);
        os << "\n";
    }
    for (const auto& entry : names.booleans)
    {
        append_feature(os, ygg::make_view(entry.index, context), entry.name);
        os << "\n";
    }
    for (const auto& entry : names.numericals)
    {
        append_feature(os, ygg::make_view(entry.index, context), entry.name);
        os << "\n";
    }
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string condition(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return fmt::format("(:{} {})", ObservationTag::keyword, get_or_create_name(names, view.get_feature()));
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string effect(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return fmt::format("(:{} {})", ObservationTag::keyword, get_or_create_name(names, view.get_feature()));
}

template<typename C>
std::string condition(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::ConditionVariant>, C> view)
{
    return ygg::visit([&](auto concrete_variant)
                      { return ygg::visit([&](auto concrete_condition) { return condition(names, concrete_condition); }, concrete_variant.get_variant()); },
                      view.get_variant());
}

template<typename C>
std::string effect(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::EffectVariant>, C> view)
{
    return ygg::visit([&](auto concrete_variant)
                      { return ygg::visit([&](auto concrete_effect) { return effect(names, concrete_effect); }, concrete_variant.get_variant()); },
                      view.get_variant());
}

template<typename Conditions>
std::vector<std::string> conditions(FeatureNames& names, Conditions values)
{
    auto result = std::vector<std::string> {};
    for (auto value : values)
        result.push_back(condition(names, value));
    return result;
}

template<typename Effects>
std::vector<std::string> effects(FeatureNames& names, Effects values)
{
    auto result = std::vector<std::string> {};
    for (auto value : values)
        result.push_back(effect(names, value));
    return result;
}

template<typename C>
std::vector<std::string> action_arguments(ygg::View<ygg::Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>>, C> view)
{
    auto result = std::vector<std::string> {};
    for (auto value : view.get_action_arguments())
        result.push_back(feature_symbol(value));
    return result;
}

template<typename C>
std::vector<std::string> call_arguments(ygg::View<ygg::Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>>, C> view)
{
    auto result = std::vector<std::string> {};
    for (const auto& value : view.get_call_arguments())
        result.push_back(ygg::visit([](auto arg) { return runir::kr::ps::ext::dl::format::expression(arg); },
                                    ygg::make_view(value, view.get_context().get_dl_repository())));
    return result;
}

template<typename Kind, typename C>
void append_rule_body(std::ostream& os, FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C> view)
{
    append_list_section(os, "conditions", conditions(names, view.get_conditions()));
    if constexpr (std::same_as<Kind, runir::kr::ps::ext::LoadTag>)
    {
        append_value_section(os, "concept", runir::kr::ps::ext::dl::format::expression(view.get_concept()));
        os << ygg::print_indent << fmt::format("(:register {})", ygg::uint_t(view.get_register().get_identifier())) << "\n";
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::SketchTag>)
    {
        append_list_section(os, "effects", effects(names, view.get_effects()));
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::DoTag>)
    {
        os << ygg::print_indent << fmt::format("(:action {})", fmt::format("{:?}", std::string(view.get_action_name().str()))) << "\n";
        append_list_section(os, "arguments", action_arguments(view));
        append_list_section(os, "effects", effects(names, view.get_effects()));
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::CallTag>)
    {
        os << ygg::print_indent << fmt::format("(:callee {})", fmt::format("{:?}", view.get_callee_name())) << "\n";
        append_list_section(os, "arguments", call_arguments(view));
    }
}

template<typename Kind, typename C>
void append_rule(std::ostream& os, FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C> view)
{
    if constexpr (std::same_as<Kind, runir::kr::ps::ext::LoadTag>)
        os << ygg::print_indent << "(:load\n";
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::SketchTag>)
        os << ygg::print_indent << "(:sketch\n";
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::DoTag>)
        os << ygg::print_indent << "(:do\n";
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::CallTag>)
        os << ygg::print_indent << "(:call\n";
    {
        ygg::IndentScope scope(os);
        if constexpr (!std::same_as<Kind, runir::kr::ps::ext::CallTag>)
        {
            os << ygg::print_indent << "(:expression\n";
            {
                ygg::IndentScope expression_scope(os);
                append_rule_body(os, names, view);
            }
            os << ygg::print_indent << ")\n";
        }
        else
        {
            append_rule_body(os, names, view);
        }
    }
    os << ygg::print_indent << ")";
}

template<typename C>
void append_rule(std::ostream& os, FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::RuleVariant>, C> view)
{
    ygg::visit([&](auto rule) { append_rule(os, names, rule); }, view.get_variant());
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string argument(ygg::View<ygg::Index<runir::kr::ps::ext::Argument<Category>>, C> view)
{
    return fmt::format("(:{} {} {})", Category::name, fmt::format("{:?}", std::string(view.get_name().str())), ygg::uint_t(view.get_identifier()));
}

template<typename C>
std::string register_(ygg::View<ygg::Index<runir::kr::ps::ext::Register>, C> view)
{
    return fmt::format("(:{} (:symbol {}))", runir::kr::dl::ConceptTag::name, ygg::uint_t(view.get_identifier()));
}

template<typename C>
std::string memory_state(ygg::View<ygg::Index<runir::kr::ps::ext::MemoryState>, C> view)
{
    return std::string(view.get_name());
}

template<typename C>
std::string rule(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::RuleVariant>, C> view)
{
    auto os = std::ostringstream {};
    append_rule(os, names, view);
    return os.str();
}

template<typename C>
std::string module(ygg::View<ygg::Index<runir::kr::ps::ext::Module>, C> view)
{
    auto names = collect_features(view);
    auto os = std::ostringstream {};

    os << "(:module\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << symbol_section(std::string(view.get_name().str())) << "\n";
        os << ygg::print_indent << "(:arguments\n";
        {
            ygg::IndentScope argument_scope(os);
            for (auto arg : view.template get_arguments<runir::kr::dl::ConceptTag>())
                os << ygg::print_indent << argument(arg) << "\n";
            for (auto arg : view.template get_arguments<runir::kr::dl::RoleTag>())
                os << ygg::print_indent << argument(arg) << "\n";
            for (auto arg : view.template get_arguments<runir::kr::dl::BooleanTag>())
                os << ygg::print_indent << argument(arg) << "\n";
            for (auto arg : view.template get_arguments<runir::kr::dl::NumericalTag>())
                os << ygg::print_indent << argument(arg) << "\n";
        }
        os << ygg::print_indent << ")\n";
        os << ygg::print_indent << "(:description \"\")\n";

        os << ygg::print_indent << "(:registers\n";
        {
            ygg::IndentScope register_scope(os);
            for (auto reg : view.get_registers())
                os << ygg::print_indent << register_(reg) << "\n";
        }
        os << ygg::print_indent << ")\n";

        os << ygg::print_indent << fmt::format("(:entry {})", memory_state(view.get_entry_memory_state())) << "\n";
        auto memory_states = std::vector<std::string> {};
        for (auto state : view.get_memory_states())
            memory_states.push_back(memory_state(state));
        append_list_section(os, "memory", memory_states);

        os << ygg::print_indent << "(:features\n";
        {
            ygg::IndentScope feature_scope(os);
            append_features(os, view.get_context(), names);
        }
        os << ygg::print_indent << ")\n";

        os << ygg::print_indent << "(:rules\n";
        {
            ygg::IndentScope transition_scope(os);
            for (const auto& transition : view.get_memory_transitions())
            {
                for (auto item : transition)
                {
                    os << ygg::print_indent << "(:rule\n";
                    {
                        ygg::IndentScope transition_scope_inner(os);
                        os << ygg::print_indent << symbol_section(std::string(item.get_symbol().str())) << "\n";
                        os << ygg::print_indent << fmt::format("(:description {})", fmt::format("{:?}", std::string(item.get_description().str()))) << "\n";
                        os << ygg::print_indent << "(:expression\n";
                        {
                            ygg::IndentScope expression_scope(os);
                            const auto source = ygg::visit([](auto rule) { return memory_state(rule.get_source()); }, item.get_variant());
                            const auto target = ygg::visit([](auto rule) { return memory_state(rule.get_target()); }, item.get_variant());
                            os << ygg::print_indent << fmt::format("(:source-memory {})", source) << "\n";
                            os << ygg::print_indent << fmt::format("(:target-memory {})", target) << "\n";
                            append_rule(os, names, item);
                            os << "\n";
                        }
                        os << ygg::print_indent << ")\n";
                    }
                    os << ygg::print_indent << ")\n";
                }
            }
        }
        os << ygg::print_indent << ")\n";
    }
    os << ")\n";
    return os.str();
}

template<typename C>
std::string module_program(ygg::View<ygg::Index<runir::kr::ps::ext::ModuleProgram>, C> view)
{
    auto os = std::ostringstream {};
    os << "(:program\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << fmt::format("(:entry {})", fmt::format("{:?}", std::string(view.get_entry_module().get_name().str()))) << "\n";
        for (auto item : view.get_modules())
        {
            append_value(os, module(item));
            os << "\n";
        }
    }
    os << ")\n";
    return os.str();
}

inline std::string module_program_proof_status(runir::kr::ps::ext::ModuleProgramProofStatus status)
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

template<tyr::planning::TaskKind Kind>
std::string module_program_proof_results(const runir::kr::ps::ext::ModuleProgramProofResults<Kind>& result)
{
    return fmt::format("ModuleProgramProofResults(status={}, graph_vertices={}, graph_edges={}, deadend_transitions={}, open_states={}, cycle={})",
                       module_program_proof_status(result.status),
                       result.graph ? result.graph->get_num_vertices() : 0,
                       result.graph ? result.graph->get_num_edges() : 0,
                       result.deadend_transitions.size(),
                       result.open_states.size(),
                       result.cycle.size());
}

}  // namespace runir::kr::ps::ext::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::Argument<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::Argument<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::argument(view), ctx); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::Register>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::Register>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::register_(view), ctx); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::MemoryState>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::MemoryState>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::memory_state(view), ctx); }
};

template<typename FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::feature(view), ctx); }
};

template<typename FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::feature(view, fmt::format("f_{}", view.get_index().get_value())), ctx);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::ConditionVariant>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::ConditionVariant>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::condition(names, view), ctx);
    }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteConditionVariant<LanguageTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteConditionVariant<LanguageTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        const auto text =
            ygg::visit([&](auto concrete_condition) { return runir::kr::ps::ext::format::condition(names, concrete_condition); }, view.get_variant());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::condition(names, view), ctx);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::EffectVariant>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::EffectVariant>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::effect(names, view), ctx);
    }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteEffectVariant<LanguageTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteEffectVariant<LanguageTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        const auto text = ygg::visit([&](auto concrete_effect) { return runir::kr::ps::ext::format::effect(names, concrete_effect); }, view.get_variant());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::effect(names, view), ctx);
    }
};

template<typename Kind, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        auto os = std::ostringstream {};
        runir::kr::ps::ext::format::append_rule(os, names, view);
        return fmt::formatter<std::string_view>::format(os.str(), ctx);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::RuleVariant>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::RuleVariant>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::rule(names, view), ctx);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::Module>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::Module>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::module(view), ctx); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::ModuleProgram>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::ModuleProgram>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::module_program(view), ctx);
    }
};

#if RUNIR_ENABLE_FMT_FORMATTERS
template<tyr::planning::TaskKind Kind>
struct fmt::formatter<runir::kr::ps::ext::ModuleProgramProofVertexLabel<Kind>> : fmt::formatter<std::string_view>
{
    auto format(const runir::kr::ps::ext::ModuleProgramProofVertexLabel<Kind>& label, format_context& ctx) const
    {
        const auto& state = label.extended_state.annotated_state;
        const auto text = fmt::format("state={} module={} initial={} goal={} alive={} unsolvable={}",
                                      ygg::uint_t(state.state.get_index()),
                                      ygg::uint_t(label.module_.get_index()),
                                      state.is_initial,
                                      state.is_goal,
                                      state.is_alive,
                                      state.is_unsolvable);
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::ext::ModuleProgramProofEdgeLabel> : fmt::formatter<std::string_view>
{
    auto format(const runir::kr::ps::ext::ModuleProgramProofEdgeLabel& label, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(label.rule ? fmt::format("rule={}", ygg::uint_t(label.rule->get_index())) : std::string("rule=<none>"),
                                                        ctx);
    }
};
#endif

template<tyr::planning::TaskKind Kind>
struct fmt::formatter<runir::kr::ps::ext::ModuleProgramProofResults<Kind>> : fmt::formatter<std::string_view>
{
    auto format(const runir::kr::ps::ext::ModuleProgramProofResults<Kind>& result, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::module_program_proof_results(result), ctx);
    }
};

#endif

#endif
