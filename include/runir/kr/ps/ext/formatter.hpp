#ifndef RUNIR_KR_PS_EXT_FORMATTER_HPP_
#define RUNIR_KR_PS_EXT_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/formatter.hpp"
#include "runir/kr/ps/ext/dl/formatter.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/kr/ps/ext/views.hpp"

#include <algorithm>
#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps::ext::format
{

inline std::string symbol_section(std::string_view value) { return fmt::format("(:symbol {})", value); }

template<typename FeatureTag>
struct NamedFeature
{
    ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>> index;
    std::string name;
};

struct FeatureNames
{
    std::vector<NamedFeature<runir::kr::dl::ConceptTag>> concepts;
    std::vector<NamedFeature<runir::kr::dl::RoleTag>> roles;
    std::vector<NamedFeature<runir::kr::ps::dl::BooleanFeature>> booleans;
    std::vector<NamedFeature<runir::kr::ps::dl::NumericalFeature>> numericals;
};

struct ReferenceNames
{
    std::vector<std::string> concept_arguments;
    std::vector<std::string> role_arguments;
    std::vector<std::string> boolean_arguments;
    std::vector<std::string> numerical_arguments;
    std::vector<std::string> concept_registers;
    std::vector<std::string> role_registers;
};

inline void set_reference_name(std::vector<std::string>& names, ygg::uint_t identifier, std::string name)
{
    if (names.size() <= identifier)
        names.resize(identifier + 1);
    names[identifier] = std::move(name);
}

inline void replace_all(std::string& text, const std::string& from, const std::string& to)
{
    auto pos = std::size_t(0);
    while ((pos = text.find(from, pos)) != std::string::npos)
    {
        text.replace(pos, from.size(), to);
        pos += to.size();
    }
}

inline void replace_identifiers(std::string& text, std::string_view constructor, const std::vector<std::string>& names)
{
    for (std::size_t i = 0; i < names.size(); ++i)
    {
        if (names[i].empty())
            continue;
        replace_all(text, fmt::format("({} {})", constructor, i), fmt::format("({} {})", constructor, names[i]));
    }
}

inline std::string expression(const ReferenceNames& refs, auto view)
{
    auto text = runir::kr::ps::ext::dl::format::expression(view);
    replace_identifiers(text, "c_argument", refs.concept_arguments);
    replace_identifiers(text, "r_argument", refs.role_arguments);
    replace_identifiers(text, "b_argument", refs.boolean_arguments);
    replace_identifiers(text, "n_argument", refs.numerical_arguments);
    replace_identifiers(text, "c_register", refs.concept_registers);
    replace_identifiers(text, "r_register", refs.role_registers);
    return text;
}

template<typename FeatureTag>
auto& named_features(FeatureNames& names)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        return names.concepts;
    else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
        return names.roles;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
        return names.booleans;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
        return names.numericals;
    else
    {
        static_assert(ygg::dependent_false<FeatureTag>::value, "unhandled feature tag in named_features");
    }
}

template<typename FeatureTag>
std::string feature_prefix()
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        return "c";
    else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
        return "r";
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
        return "b";
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
        return "n";
    else
    {
        static_assert(ygg::dependent_false<FeatureTag>::value, "unhandled feature tag in feature_prefix");
    }
}

template<typename FeatureTag, typename C>
std::string feature_symbol(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> feature)
{
    return ygg::visit([](auto concrete_feature) { return std::string(concrete_feature.get_symbol()); }, feature.get_variant());
}

template<typename FeatureTag, typename C>
std::string get_or_create_name(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> feature)
{
    auto& entries = named_features<FeatureTag>(names);
    for (const auto& entry : entries)
        if (entry.index == feature.get_index())
            return entry.name;

    auto name = feature_symbol(feature);
    entries.push_back(NamedFeature<FeatureTag> { feature.get_index(), name });
    return name;
}

template<runir::kr::dl::CategoryTag Category>
struct CallFeatureTag
{
    using Type = Category;
};

template<>
struct CallFeatureTag<runir::kr::dl::BooleanTag>
{
    using Type = runir::kr::ps::dl::BooleanFeature;
};

template<>
struct CallFeatureTag<runir::kr::dl::NumericalTag>
{
    using Type = runir::kr::ps::dl::NumericalFeature;
};

template<runir::kr::dl::CategoryTag Category, typename C>
std::string call_argument_name(FeatureNames& names, ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, Category>> argument, const C& context)
{
    using FeatureTag = typename CallFeatureTag<Category>::Type;
    for (const auto& entry : named_features<FeatureTag>(names))
    {
        const auto feature = ygg::make_view(entry.index, context);
        const auto matches = ygg::visit([&](auto concrete_feature) { return concrete_feature.get_feature().get_index() == argument; }, feature.get_variant());
        if (matches)
            return entry.name;
    }

    throw std::runtime_error("Call-rule arguments must reference declared features by symbol.");
}

template<typename FeatureTag>
std::string feature_type()
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        return runir::kr::dl::ConceptTag::name;
    else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
        return runir::kr::dl::RoleTag::name;
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

inline bool is_atomic_list_value(std::string_view value) { return !value.empty() && value.front() != '(' && value.find('\n') == std::string_view::npos; }

inline void append_list_section(std::ostream& os, std::string_view name, const std::vector<std::string>& values)
{
    if (values.empty())
    {
        os << ygg::print_indent << fmt::format("(:{})", name) << "\n";
        return;
    }

    const auto all_atomic = std::ranges::all_of(values, [](const auto& value) { return is_atomic_list_value(value); });
    if (all_atomic || (values.size() == 1 && values.front().find('\n') == std::string::npos))
    {
        os << ygg::print_indent << fmt::format("(:{} {})", name, fmt::join(values, " ")) << "\n";
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
    const auto formatted_value = runir::pretty_sexpression(value);
    if (formatted_value.find('\n') == std::string::npos)
    {
        os << ygg::print_indent << fmt::format("(:{} {})", name, formatted_value) << "\n";
        return;
    }

    os << ygg::print_indent << fmt::format("(:{}\n", name);
    {
        ygg::IndentScope scope(os);
        append_value(os, formatted_value);
        os << "\n";
    }
    os << ygg::print_indent << ")\n";
}

template<typename FeatureTag, typename C>
void append_feature(std::ostream& os,
                    const ReferenceNames& refs,
                    ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view,
                    std::string_view)
{
    os << ygg::print_indent << fmt::format("(:{}\n", feature_type<FeatureTag>());
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << symbol_section(std::string(view.get_symbol().str())) << "\n";
        append_value_section(os, "expression", expression(refs, view.get_feature()));
    }
    os << ygg::print_indent << ")";
}

template<typename FeatureTag, typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view, std::string_view name)
{
    auto refs = ReferenceNames {};
    auto os = std::ostringstream {};
    append_feature(os, refs, view, name);
    return os.str();
}

template<typename FeatureTag, typename C>
void append_feature(std::ostream& os,
                    const ReferenceNames& refs,
                    ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> view,
                    std::string_view name)
{
    ygg::visit([&](auto concrete_feature) { append_feature(os, refs, concrete_feature, name); }, view.get_variant());
}

template<typename FeatureTag, typename C>
void append_feature(std::ostream& os,
                    FeatureNames& names,
                    const ReferenceNames& refs,
                    ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> view)
{
    append_feature(os, refs, view, get_or_create_name(names, view));
}

template<typename FeatureTag, typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> view, std::string_view name)
{
    auto refs = ReferenceNames {};
    auto os = std::ostringstream {};
    append_feature(os, refs, view, name);
    return os.str();
}

template<typename FeatureTag, typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> view)
{
    return feature(view, fmt::format("{}_{}", feature_prefix<FeatureTag>(), view.get_index().get_value()));
}

template<typename Features>
void append_declared_features(std::ostream& os, FeatureNames& names, const ReferenceNames& refs, Features features)
{
    for (auto feature : features)
    {
        auto feature_os = std::ostringstream {};
        append_feature(feature_os, names, refs, feature);
        append_value(os, feature_os.str());
        os << "\n";
    }
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string condition(FeatureNames& names,
                      ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return fmt::format("({} {})", ObservationTag::keyword, get_or_create_name(names, view.get_feature()));
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string effect(FeatureNames& names,
                   ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view)
{
    return fmt::format("({} {})", ObservationTag::keyword, get_or_create_name(names, view.get_feature()));
}

template<typename C>
std::string condition(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C> view)
{
    return ygg::visit([&](auto concrete_variant)
                      { return ygg::visit([&](auto concrete_condition) { return condition(names, concrete_condition); }, concrete_variant.get_variant()); },
                      view.get_variant());
}

template<typename C>
std::string effect(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>, C> view)
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
std::vector<std::string> call_arguments(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>>, C> view)
{
    auto result = std::vector<std::string> {};
    for (const auto& value : view.get_call_arguments())
        value.apply([&](auto arg) { result.push_back(call_argument_name(names, arg, view.get_context())); });
    return result;
}

template<runir::kr::ps::ext::RuleKind Kind, typename C>
void append_rule_body(std::ostream& os, FeatureNames& names, const ReferenceNames& refs, ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C> view)
{
    append_list_section(os, "conditions", conditions(names, view.get_conditions()));
    if constexpr (std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::ConceptTag>>
                  || std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::RoleTag>>)
    {
        constexpr auto category_name =
            std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::ConceptTag>> ? runir::kr::dl::ConceptTag::name : runir::kr::dl::RoleTag::name;
        append_value_section(os, category_name, expression(refs, view.get_expression()));
        os << ygg::print_indent << "(:register\n";
        {
            ygg::IndentScope register_scope(os);
            os << ygg::print_indent << fmt::format("(:{} {})", category_name, std::string(view.get_register().get_name().str())) << "\n";
        }
        os << ygg::print_indent << ")\n";
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
        os << ygg::print_indent << fmt::format("(:callee {})", view.get_callee_name()) << "\n";
        append_list_section(os, "arguments", call_arguments(names, view));
    }
}

template<runir::kr::ps::ext::RuleKind Kind, typename C>
void append_rule(std::ostream& os, FeatureNames& names, const ReferenceNames& refs, ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C> view)
{
    if constexpr (std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::ConceptTag>>
                  || std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::RoleTag>>)
        os << ygg::print_indent << "(:load\n";
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::SketchTag>)
        os << ygg::print_indent << "(:sketch\n";
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::DoTag>)
        os << ygg::print_indent << "(:do\n";
    else if constexpr (std::same_as<Kind, runir::kr::ps::ext::CallTag>)
        os << ygg::print_indent << "(:call\n";
    {
        ygg::IndentScope scope(os);
        append_rule_body(os, names, refs, view);
    }
    os << ygg::print_indent << ")";
}

template<typename C>
void append_rule(std::ostream& os, FeatureNames& names, const ReferenceNames& refs, ygg::View<ygg::Index<runir::kr::ps::ext::RuleVariant>, C> view)
{
    ygg::visit([&](auto rule) { append_rule(os, names, refs, rule); }, view.get_variant());
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string argument(ygg::View<ygg::Index<runir::kr::ps::ext::Argument<Category>>, C> view)
{
    return fmt::format("(:{} {})", Category::name, std::string(view.get_name().str()));
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string register_(ygg::View<ygg::Index<runir::kr::ps::ext::Register<Category>>, C> view)
{
    return fmt::format("(:{} {})", Category::name, std::string(view.get_name().str()));
}

template<typename C>
std::string memory_state(ygg::View<ygg::Index<runir::kr::ps::ext::MemoryState>, C> view)
{
    return std::string(view.get_name());
}

template<typename C>
std::string rule(FeatureNames& names, ygg::View<ygg::Index<runir::kr::ps::ext::RuleVariant>, C> view)
{
    auto refs = ReferenceNames {};
    auto os = std::ostringstream {};
    append_rule(os, names, refs, view);
    return os.str();
}

template<typename C>
std::string module(ygg::View<ygg::Index<runir::kr::ps::ext::Module>, C> view)
{
    auto names = FeatureNames {};
    auto refs = ReferenceNames {};
    auto os = std::ostringstream {};

    os << "(:module\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << symbol_section(std::string(view.get_name().str())) << "\n";
        auto arguments = std::vector<std::string> {};
        for (auto arg : view.template get_arguments<runir::kr::dl::ConceptTag>())
        {
            set_reference_name(refs.concept_arguments, ygg::uint_t(arg.get_identifier()), std::string(arg.get_name().str()));
            arguments.push_back(argument(arg));
        }
        for (auto arg : view.template get_arguments<runir::kr::dl::RoleTag>())
        {
            set_reference_name(refs.role_arguments, ygg::uint_t(arg.get_identifier()), std::string(arg.get_name().str()));
            arguments.push_back(argument(arg));
        }
        for (auto arg : view.template get_arguments<runir::kr::dl::BooleanTag>())
        {
            set_reference_name(refs.boolean_arguments, ygg::uint_t(arg.get_identifier()), std::string(arg.get_name().str()));
            arguments.push_back(argument(arg));
        }
        for (auto arg : view.template get_arguments<runir::kr::dl::NumericalTag>())
        {
            set_reference_name(refs.numerical_arguments, ygg::uint_t(arg.get_identifier()), std::string(arg.get_name().str()));
            arguments.push_back(argument(arg));
        }
        append_list_section(os, "arguments", arguments);

        auto registers = std::vector<std::string> {};
        for (auto reg : view.template get_registers<runir::kr::dl::ConceptTag>())
        {
            set_reference_name(refs.concept_registers, ygg::uint_t(reg.get_identifier()), std::string(reg.get_name().str()));
            registers.push_back(register_(reg));
        }
        for (auto reg : view.template get_registers<runir::kr::dl::RoleTag>())
        {
            set_reference_name(refs.role_registers, ygg::uint_t(reg.get_identifier()), std::string(reg.get_name().str()));
            registers.push_back(register_(reg));
        }
        append_list_section(os, "registers", registers);

        os << ygg::print_indent << fmt::format("(:entry {})", memory_state(view.get_entry_memory_state())) << "\n";
        auto memory_states = std::vector<std::string> {};
        for (auto state : view.get_memory_states())
            memory_states.push_back(memory_state(state));
        append_list_section(os, "memory", memory_states);

        os << ygg::print_indent << "(:features\n";
        {
            ygg::IndentScope feature_scope(os);
            append_declared_features(os, names, refs, view.template get_features<runir::kr::dl::ConceptTag>());
            append_declared_features(os, names, refs, view.template get_features<runir::kr::dl::RoleTag>());
            append_declared_features(os, names, refs, view.template get_features<runir::kr::ps::dl::BooleanFeature>());
            append_declared_features(os, names, refs, view.template get_features<runir::kr::ps::dl::NumericalFeature>());
        }
        os << ygg::print_indent << ")\n";

        os << ygg::print_indent << "(:rules\n";
        {
            ygg::IndentScope transition_scope(os);
            for (const auto& transition : view.get_memory_transitions())
            {
                for (auto item : transition)
                {
                    auto rule_item_os = std::ostringstream {};
                    rule_item_os << "(:rule\n";
                    {
                        ygg::IndentScope transition_scope_inner(rule_item_os);
                        rule_item_os << ygg::print_indent << symbol_section(std::string(item.get_symbol().str())) << "\n";
                        rule_item_os << ygg::print_indent << "(:expression\n";
                        {
                            ygg::IndentScope expression_scope(rule_item_os);
                            const auto source = ygg::visit([](auto rule) { return memory_state(rule.get_source()); }, item.get_variant());
                            const auto target = ygg::visit([](auto rule) { return memory_state(rule.get_target()); }, item.get_variant());
                            rule_item_os << ygg::print_indent << fmt::format("(:source-memory {})", source) << "\n";
                            rule_item_os << ygg::print_indent << fmt::format("(:target-memory {})", target) << "\n";
                            auto rule_os = std::ostringstream {};
                            append_rule(rule_os, names, refs, item);
                            append_value(rule_item_os, rule_os.str());
                            rule_item_os << "\n";
                        }
                        rule_item_os << ygg::print_indent << ")\n";
                    }
                    rule_item_os << ")";
                    append_value(os, rule_item_os.str());
                    os << "\n";
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
        os << ygg::print_indent << fmt::format("(:entry {})", std::string(view.get_entry_module().get_name().str())) << "\n";
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

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::Register<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::Register<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::register_(view), ctx); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::MemoryState>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::MemoryState>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::memory_state(view), ctx); }
};

template<>
struct fmt::formatter<runir::kr::ps::ext::InternalMemoryState> : fmt::formatter<std::string_view>
{
    auto format(const runir::kr::ps::ext::InternalMemoryState& state, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(fmt::format("internal({})", state.value), ctx);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::ext::ExternalMemoryState> : fmt::formatter<std::string_view>
{
    auto format(const runir::kr::ps::ext::ExternalMemoryState& state, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(fmt::format("external({})", state.value), ctx);
    }
};

template<typename FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::feature(view), ctx); }
};

template<typename FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::feature(view, fmt::format("f_{}", view.get_index().get_value())), ctx);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::condition(names, view), ctx);
    }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, LanguageTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, LanguageTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        const auto text =
            ygg::visit([&](auto concrete_condition) { return runir::kr::ps::ext::format::condition(names, concrete_condition); }, view.get_variant());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, LanguageTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, LanguageTag, FeatureTag, ObservationTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::condition(names, view), ctx);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::effect(names, view), ctx);
    }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, LanguageTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, LanguageTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        const auto text = ygg::visit([&](auto concrete_effect) { return runir::kr::ps::ext::format::effect(names, concrete_effect); }, view.get_variant());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, LanguageTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, LanguageTag, FeatureTag, ObservationTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        return fmt::formatter<std::string_view>::format(runir::kr::ps::ext::format::effect(names, view), ctx);
    }
};

template<runir::kr::ps::ext::RuleKind Kind, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ext::Rule<Kind>>, C>;
    auto format(View view, format_context& ctx) const
    {
        auto names = runir::kr::ps::ext::format::FeatureNames {};
        auto os = std::ostringstream {};
        runir::kr::ps::ext::format::append_rule(os, names, runir::kr::ps::ext::format::ReferenceNames {}, view);
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
