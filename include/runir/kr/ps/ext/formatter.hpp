#ifndef RUNIR_KR_PS_EXT_FORMATTER_HPP_
#define RUNIR_KR_PS_EXT_FORMATTER_HPP_

#include "runir/common/config.hpp"
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
#include <tyr/common/iostream.hpp>
#include <tyr/common/types.hpp>
#include <vector>

namespace runir::kr::ps::ext::format
{

template<typename String>
std::string quoted(const String& value)
{
    return fmt::format("\"{}\"", value.str());
}

inline std::string quoted(std::string_view value) { return fmt::format("\"{}\"", value); }
inline std::string quoted(const std::string& value) { return quoted(std::string_view(value)); }

template<typename FeatureTag>
struct NamedFeature
{
    tyr::Index<runir::kr::ps::ext::Feature<FeatureTag>> index;
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
std::string get_or_create_name(FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C> feature)
{
    auto& entries = named_features<FeatureTag>(names);
    for (const auto& entry : entries)
        if (entry.index == feature.get_index())
            return entry.name;

    auto name = fmt::format("{}_{}", feature_prefix<FeatureTag>(), entries.size());
    entries.push_back(NamedFeature<FeatureTag> { feature.get_index(), name });
    return name;
}

template<typename FeatureTag, typename ObservationTag, typename C>
void collect_feature(FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    get_or_create_name(names, view.get_feature());
}

template<typename FeatureTag, typename ObservationTag, typename C>
void collect_feature(FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    get_or_create_name(names, view.get_feature());
}

template<typename C>
void collect_features(FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::ConditionVariant>, C> view)
{
    tyr::visit([&](auto concrete_variant)
               { tyr::visit([&](auto concrete_condition) { collect_feature(names, concrete_condition); }, concrete_variant.get_variant()); },
               view.get_variant());
}

template<typename C>
void collect_features(FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::EffectVariant>, C> view)
{
    tyr::visit([&](auto concrete_variant)
               { tyr::visit([&](auto concrete_effect) { collect_feature(names, concrete_effect); }, concrete_variant.get_variant()); },
               view.get_variant());
}

template<typename Kind, typename C>
void collect_features(FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::Rule<Kind>>, C> view)
{
    for (auto condition : view.get_conditions())
        collect_features(names, condition);
    if constexpr (std::same_as<Kind, runir::kr::ps::ext::SketchTag>)
        for (auto effect : view.get_effects())
            collect_features(names, effect);
}

template<typename C>
void collect_features(FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::RuleVariant>, C> view)
{
    tyr::visit([&](auto rule) { collect_features(names, rule); }, view.get_variant());
}

template<typename C>
FeatureNames collect_features(tyr::View<tyr::Index<runir::kr::ps::ext::Module>, C> view)
{
    auto names = FeatureNames {};
    for (const auto& transition : view.get_memory_transitions())
        for (auto rule : tyr::make_view(transition.rules, view.get_context()))
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

template<typename FeatureTag, typename C>
std::string feature(tyr::View<tyr::Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C> view, std::string_view name)
{
    return fmt::format("({} {} {} {} {})",
                       feature_type<FeatureTag>(),
                       name,
                       quoted(view.get_symbol()),
                       quoted(view.get_description()),
                       runir::kr::ps::ext::dl::format::expression(view.get_feature()));
}

template<typename FeatureTag, typename C>
std::string feature(tyr::View<tyr::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C> view, std::string_view name)
{
    return tyr::visit([&](auto concrete_feature) { return feature(concrete_feature, name); }, view.get_variant());
}

template<typename FeatureTag, typename C>
std::string feature(tyr::View<tyr::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C> view)
{
    return feature(view, fmt::format("{}_{}", feature_prefix<FeatureTag>(), view.get_index().get_value()));
}

template<typename C>
void append_features(std::ostream& os, const C& context, const FeatureNames& names)
{
    for (const auto& entry : names.concepts)
        os << tyr::print_indent << feature(tyr::make_view(entry.index, context), entry.name) << "\n";
    for (const auto& entry : names.booleans)
        os << tyr::print_indent << feature(tyr::make_view(entry.index, context), entry.name) << "\n";
    for (const auto& entry : names.numericals)
        os << tyr::print_indent << feature(tyr::make_view(entry.index, context), entry.name) << "\n";
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string condition(FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return fmt::format("({} {})", ObservationTag::keyword, get_or_create_name(names, view.get_feature()));
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string effect(FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return fmt::format("({} {})", ObservationTag::keyword, get_or_create_name(names, view.get_feature()));
}

template<typename C>
std::string condition(FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::ConditionVariant>, C> view)
{
    return tyr::visit([&](auto concrete_variant)
                      { return tyr::visit([&](auto concrete_condition) { return condition(names, concrete_condition); }, concrete_variant.get_variant()); },
                      view.get_variant());
}

template<typename C>
std::string effect(FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::EffectVariant>, C> view)
{
    return tyr::visit([&](auto concrete_variant)
                      { return tyr::visit([&](auto concrete_effect) { return effect(names, concrete_effect); }, concrete_variant.get_variant()); },
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
std::vector<std::string> action_arguments(tyr::View<tyr::Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>>, C> view)
{
    auto result = std::vector<std::string> {};
    for (auto value : view.get_action_arguments())
        result.push_back(runir::kr::ps::ext::dl::format::expression(value));
    return result;
}

template<typename C>
std::vector<std::string> call_arguments(tyr::View<tyr::Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>>, C> view)
{
    auto result = std::vector<std::string> {};
    for (const auto& value : view.get_call_arguments())
        result.push_back(tyr::visit([](auto arg) { return runir::kr::ps::ext::dl::format::expression(arg); },
                                    tyr::make_view(value, view.get_context().get_dl_repository())));
    return result;
}

template<typename Kind, typename C>
void append_rule_body(std::ostream& os, FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::Rule<Kind>>, C> view)
{
    os << tyr::print_indent << fmt::format("(:conditions {})", fmt::join(conditions(names, view.get_conditions()), " ")) << "\n";
    if constexpr (std::same_as<Kind, runir::kr::ps::ext::LoadTag>)
    {
        os << tyr::print_indent << fmt::format("(:concept {})", runir::kr::ps::ext::dl::format::expression(view.get_concept())) << "\n";
        os << tyr::print_indent << fmt::format("(:register {})", quoted(view.get_register().get_name())) << "\n";
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::SketchTag>)
    {
        os << tyr::print_indent << fmt::format("(:effects {})", fmt::join(effects(names, view.get_effects()), " ")) << "\n";
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::DoTag>)
    {
        os << tyr::print_indent << fmt::format("(:action {})", quoted(view.get_action_name())) << "\n";
        os << tyr::print_indent << fmt::format("(:arguments {})", fmt::join(action_arguments(view), " ")) << "\n";
    }
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::CallTag>)
    {
        os << tyr::print_indent << fmt::format("(:callee {})", quoted(view.get_callee_name())) << "\n";
        os << tyr::print_indent << fmt::format("(:arguments {})", fmt::join(call_arguments(view), " ")) << "\n";
    }
}

template<typename Kind, typename C>
void append_rule(std::ostream& os, FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::Rule<Kind>>, C> view)
{
    if constexpr (std::same_as<Kind, runir::kr::ps::ext::LoadTag>)
        os << tyr::print_indent << "(load\n";
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::SketchTag>)
        os << tyr::print_indent << "(sketch\n";
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::DoTag>)
        os << tyr::print_indent << "(do\n";
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::CallTag>)
        os << tyr::print_indent << "(call\n";
    {
        tyr::IndentScope scope(os);
        append_rule_body(os, names, view);
    }
    os << tyr::print_indent << ")";
}

template<typename C>
void append_rule(std::ostream& os, FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::RuleVariant>, C> view)
{
    tyr::visit([&](auto rule) { append_rule(os, names, rule); }, view.get_variant());
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string argument(tyr::View<tyr::Index<runir::kr::ps::ext::Argument<Category>>, C> view)
{
    return fmt::format("({} {} {})", Category::name, quoted(view.get_name()), tyr::uint_t(view.get_identifier()));
}

template<typename C>
std::string register_(tyr::View<tyr::Index<runir::kr::ps::ext::Register>, C> view)
{
    return fmt::format("({} {} {})", runir::kr::dl::ConceptTag::name, quoted(view.get_name()), tyr::uint_t(view.get_identifier()));
}

template<typename C>
std::string memory_state(tyr::View<tyr::Index<runir::kr::ps::ext::MemoryState>, C> view)
{
    return quoted(view.get_name());
}

template<typename C>
std::string rule(FeatureNames& names, tyr::View<tyr::Index<runir::kr::ps::ext::RuleVariant>, C> view)
{
    auto os = std::ostringstream {};
    append_rule(os, names, view);
    return os.str();
}

template<typename C>
std::string module(tyr::View<tyr::Index<runir::kr::ps::ext::Module>, C> view)
{
    auto names = collect_features(view);
    auto os = std::ostringstream {};

    os << fmt::format("(module {}\n", quoted(view.get_name()));
    {
        tyr::IndentScope scope(os);
        os << tyr::print_indent << "(:arguments\n";
        {
            tyr::IndentScope argument_scope(os);
            for (auto arg : view.template get_arguments<runir::kr::dl::ConceptTag>())
                os << tyr::print_indent << argument(arg) << "\n";
            for (auto arg : view.template get_arguments<runir::kr::dl::RoleTag>())
                os << tyr::print_indent << argument(arg) << "\n";
            for (auto arg : view.template get_arguments<runir::kr::dl::BooleanTag>())
                os << tyr::print_indent << argument(arg) << "\n";
            for (auto arg : view.template get_arguments<runir::kr::dl::NumericalTag>())
                os << tyr::print_indent << argument(arg) << "\n";
        }
        os << tyr::print_indent << ")\n";

        os << tyr::print_indent << "(:registers\n";
        {
            tyr::IndentScope register_scope(os);
            for (auto reg : view.get_registers())
                os << tyr::print_indent << register_(reg) << "\n";
        }
        os << tyr::print_indent << ")\n";

        os << tyr::print_indent << fmt::format("(:entry {})", memory_state(view.get_entry_memory_state())) << "\n";
        os << tyr::print_indent
           << fmt::format("(:memory {})",
                          fmt::join(
                              [&]
                              {
                                  auto values = std::vector<std::string> {};
                                  for (auto state : view.get_memory_states())
                                      values.push_back(memory_state(state));
                                  return values;
                              }(),
                              " "))
           << "\n";

        os << tyr::print_indent << "(:features\n";
        {
            tyr::IndentScope feature_scope(os);
            append_features(os, view.get_context(), names);
        }
        os << tyr::print_indent << ")\n";

        os << tyr::print_indent << "(:transitions\n";
        {
            tyr::IndentScope transition_scope(os);
            for (const auto& transition : view.get_memory_transitions())
            {
                os << tyr::print_indent
                   << fmt::format("({} {}\n",
                                  memory_state(tyr::make_view(transition.source, view.get_context())),
                                  memory_state(tyr::make_view(transition.target, view.get_context())));
                {
                    tyr::IndentScope rule_scope(os);
                    for (auto item : tyr::make_view(transition.rules, view.get_context()))
                    {
                        append_rule(os, names, item);
                        os << "\n";
                    }
                }
                os << tyr::print_indent << ")\n";
            }
        }
        os << tyr::print_indent << ")\n";
    }
    os << ")\n";
    return os.str();
}

template<typename C>
std::string module_program(tyr::View<tyr::Index<runir::kr::ps::ext::ModuleProgram>, C> view)
{
    auto os = std::ostringstream {};
    os << "(program\n";
    {
        tyr::IndentScope scope(os);
        os << tyr::print_indent << fmt::format("(:entry {})", quoted(view.get_entry_module().get_name())) << "\n";
        for (auto item : view.get_modules())
            os << tyr::print_indent << module(item);
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
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::Argument<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::Argument<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::argument(view), ctx); }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::Register>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::Register>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::register_(view), ctx); }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::MemoryState>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::MemoryState>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::memory_state(view), ctx); }
};

template<typename FeatureTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::feature(view), ctx); }
};

template<typename FeatureTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::feature(view, fmt::format("f_{}", view.get_index().get_value())), ctx);
    }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::ConditionVariant>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::ConditionVariant>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::condition(names, view), ctx);
    }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::ConcreteConditionVariant<LanguageTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::ConcreteConditionVariant<LanguageTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        const auto text =
            tyr::visit([&](auto concrete_condition) { return runir::kr::ps::ext::format::condition(names, concrete_condition); }, view.get_variant());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::EffectVariant>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::EffectVariant>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::effect(names, view), ctx);
    }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::ConcreteEffectVariant<LanguageTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::ConcreteEffectVariant<LanguageTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        const auto text = tyr::visit([&](auto concrete_effect) { return runir::kr::ps::ext::format::effect(names, concrete_effect); }, view.get_variant());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename Kind, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::Rule<Kind>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::Rule<Kind>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        auto os = std::ostringstream {};
        runir::kr::ps::ext::format::append_rule(os, names, view);
        return fmt::formatter<std::string_view>::format(os.str(), ctx);
    }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::RuleVariant>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::RuleVariant>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::rule(names, view), ctx);
    }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::Module>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::Module>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::module(view), ctx); }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ext::ModuleProgram>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ext::ModuleProgram>, C>;
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
                                      tyr::uint_t(state.state.get_index()),
                                      tyr::uint_t(label.module_.get_index()),
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
        return fmt::formatter<std::string_view>::format(label.rule ? fmt::format("rule={}", tyr::uint_t(label.rule->get_index())) :
                                                                     std::string("rule=<none>"),
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
