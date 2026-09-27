#ifndef RUNIR_KR_PS_ICP_FORMATTER_HPP_
#define RUNIR_KR_PS_ICP_FORMATTER_HPP_

#include "runir/kr/dl/semantics/formatter.hpp"
#include "runir/kr/ps/icp/module_view.hpp"
#include "runir/kr/ps/icp/repository.hpp"
#include "runir/kr/ps/icp/rule_view.hpp"
#include "runir/kr/ps/icp/views.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <tyr/formalism/planning/formatter.hpp>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps::icp
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
void append_feature(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::IcpFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view)
{
    os << ygg::print_indent << "(:" << feature_type<FeatureTag>() << "\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << "(:symbol " << view.get_symbol() << ")\n";
        os << ygg::print_indent << "(:expression ";
        fmt::format_to(std::ostream_iterator<char>(os), "{}", view.get_expression());
        os << ")\n";
    }
    os << ygg::print_indent << ')';
}

template<typename FeatureTag, typename C>
void append_feature(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, FeatureTag>>, C> view)
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
Out condition(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::IcpFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view, Out out)
{
    return fmt::format_to(out, "({} {})", ObservationTag::keyword, view.get_feature().get_symbol());
}

template<typename Out, typename LanguageTag, typename C>
Out condition(ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::IcpFamilyTag, LanguageTag>>, C> view, Out out)
{
    return ygg::visit([&](auto concrete) { return condition(concrete, out); }, view.get_variant());
}

template<typename Out, typename C>
Out condition(ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::IcpFamilyTag>>, C> view, Out out)
{
    return ygg::visit([&](auto concrete) { return condition(concrete, out); }, view.get_variant());
}

template<typename Out, typename FeatureTag, typename ObservationTag, typename C>
Out effect(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view, Out out)
{
    return fmt::format_to(out, "({} {})", ObservationTag::keyword, view.get_feature().get_symbol());
}

template<typename Out, typename LanguageTag, typename C>
Out effect(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::IcpFamilyTag, LanguageTag>>, C> view, Out out)
{
    return ygg::visit([&](auto concrete) { return effect(concrete, out); }, view.get_variant());
}

template<typename Out, typename C>
Out effect(ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::IcpFamilyTag>>, C> view, Out out)
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

template<typename Predicate, typename ArgumentNames>
void append_indexical(std::ostream& os, Predicate predicate, const ArgumentNames& arguments)
{
    const auto operation = predicate.get_operation();
    if constexpr (std::same_as<decltype(operation), const ConditionOperation>)
        os << '(' << (operation == ConditionOperation::BELONGS ? "belongs" : "not-belongs");
    else
        os << '(' << (operation == EffectOperation::ENTER ? "enter" : "exit");
    predicate.get_object_reference().apply(
        [&](const auto& reference)
        {
            if constexpr (std::same_as<std::remove_cvref_t<decltype(reference)>, ArgumentPosition>)
            {
                os << " (:argument ";
                if (reference.value < arguments.size())
                    os << arguments.at(reference.value);
                else
                    os << "arg" << reference.value;
                os << ')';
            }
            else
                os << " (:register (:concept " << ygg::make_view(reference, predicate.get_context().get_dl_repository()).get_name() << "))";
        });
    os << " (:concept " << predicate.get_concept_feature().get_symbol() << "))";
}

template<runir::kr::ps::icp::RuleKind Kind, typename C>
void append_rule_body(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::icp::Rule<Kind>>, C> view)
{
    if constexpr (std::same_as<Kind, CruleTag>)
    {
        os << ygg::print_indent << "(:action " << fmt::format("{:?}", view.get_action_name().view()) << ")\n";
        append_inline_section(os, "arguments", view.get_argument_names(), [](auto& out, const auto& name) { out << name; });
        append_conditions(os, view.get_conditions());
        append_inline_section(os,
                              "xconditions",
                              view.get_xconditions(),
                              [&](auto& out, auto predicate) { append_indexical(out, predicate, view.get_argument_names()); });
        append_inline_section(os,
                              "xeffects",
                              view.get_xeffects(),
                              [&](auto& out, auto predicate) { append_indexical(out, predicate, view.get_argument_names()); });
        append_effects(os, view.get_effects());
    }
    else
    {
        append_conditions(os, view.get_conditions());
        using Category = typename Kind::Category;
        os << ygg::print_indent << "(:" << Category::name << ' ' << view.get_feature().get_symbol() << ")\n";
        os << ygg::print_indent << "(:register (:" << Category::name << ' ' << view.get_register().get_name() << "))\n";
        append_effects(os, view.get_effects());
    }
}

template<runir::kr::ps::icp::RuleKind Kind, typename C>
void append_rule(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::icp::Rule<Kind>>, C> view)
{
    os << ygg::print_indent << "(:" << Kind::keyword << "\n";
    {
        ygg::IndentScope scope(os);
        append_rule_body(os, view);
    }
    os << ygg::print_indent << ')';
}

template<typename C>
void append_rule(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>>, C> view)
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
void append_module(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::icp::Module>, C> view)
{
    os << ygg::print_indent << "(:module\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << "(:symbol " << view.get_name() << ")\n";

        os << ygg::print_indent << "(:arguments)\n";

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
        os << ygg::print_indent << "(:reset-spo";
        for (const auto& pair : view.get_reset_pairs())
            os << " (:prec " << ygg::make_view(pair.before, view.get_context()).get_symbol() << ' '
               << ygg::make_view(pair.after, view.get_context()).get_symbol() << ')';
        os << ")\n";
    }
    os << ygg::print_indent << ')';
}

template<typename C>
std::string module(ygg::View<ygg::Index<runir::kr::ps::icp::Module>, C> view)
{
    auto os = std::ostringstream {};
    append_module(os, view);
    return os.str();
}

template<typename C>
void append_program(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::icp::Program>, C> view)
{
    os << ygg::print_indent << "(:program\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << "(:entry " << view.get_entry_module().get_name() << ")\n";
        append_module(os, view.get_module());
        os << '\n';
    }
    os << ygg::print_indent << ')';
}

template<typename C>
std::string program(ygg::View<ygg::Index<runir::kr::ps::icp::Program>, C> view)
{
    auto os = std::ostringstream {};
    append_program(os, view);
    return os.str();
}

}  // namespace runir::kr::ps::icp

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::icp::MemoryState>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return fmt::format_to(context.out(), "{}", view.get_name()); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::icp::ModuleSymbol>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return fmt::format_to(context.out(), "{}", view.get_name()); }
};

template<typename FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, FeatureTag>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::icp::feature(view), context); }
};

template<typename FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::IcpFamilyTag, runir::kr::DlTag, FeatureTag>>, C>> :
    fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::icp::feature(view), context); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::IcpFamilyTag>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return runir::kr::ps::icp::condition(view, context.out()); }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::IcpFamilyTag, LanguageTag>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return runir::kr::ps::icp::condition(view, context.out()); }
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::IcpFamilyTag, LanguageTag, FeatureTag, ObservationTag>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return runir::kr::ps::icp::condition(view, context.out()); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::IcpFamilyTag>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return runir::kr::ps::icp::effect(view, context.out()); }
};

template<typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::IcpFamilyTag, LanguageTag>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return runir::kr::ps::icp::effect(view, context.out()); }
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, LanguageTag, FeatureTag, ObservationTag>>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return runir::kr::ps::icp::effect(view, context.out()); }
};

template<runir::kr::ps::icp::RuleKind Kind, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::icp::Rule<Kind>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::icp::rule(view), context); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::icp::rule(view), context); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::icp::Module>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::icp::module(view), context); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::icp::Program>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::icp::program(view), context); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::icp::XCondition>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const
    {
        std::ostringstream os;
        runir::kr::ps::icp::append_indexical(os, view, std::vector<std::string> {});
        return fmt::formatter<std::string_view>::format(os.str(), context);
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::icp::XEffect>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const
    {
        std::ostringstream os;
        runir::kr::ps::icp::append_indexical(os, view, std::vector<std::string> {});
        return fmt::formatter<std::string_view>::format(os.str(), context);
    }
};

#endif
