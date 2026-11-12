#ifndef RUNIR_KR_PS_FORMATTER_HPP_
#define RUNIR_KR_PS_FORMATTER_HPP_

#include "runir/kr/dl/semantics/formatter.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/memory_state_view.hpp"
#include "runir/kr/ps/module_symbol_view.hpp"
#include "runir/kr/ps/program_proof_graph.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <algorithm>
#include <concepts>
#include <fmt/format.h>
#include <iterator>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps
{

template<runir::kr::ps::dl::FeatureTag FeatureTag>
constexpr std::string_view feature_type()
{
    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::QueryFeature>)
        return FeatureTag::keyword;
    else
        return FeatureTag::name;
}

namespace detail
{
/// The head of a q_head with `arity` columns; none for any other query.
template<typename C>
std::optional<std::string> query_head(urd::index_view<runir::kr::dl::Query<runir::kr::dl::QueryHeadTag>, C> view, size_t arity)
{
    if (view.get_columns().size() != arity)
        return std::nullopt;
    return std::string(view.get_relation().get_name());
}
template<typename V>
std::optional<std::string> query_head(V, size_t)
{ return std::nullopt; }
template<typename C>
std::optional<std::string> query_head(urd::index_view<runir::kr::dl::Query<>, C> view, size_t arity)
{
    return std::visit([&](auto concrete) { return query_head(concrete, arity); }, view.get_variant());
}

/// The head that a feature's expression reads whole, as `(:head h)` parses into (parse_head_feature in
/// src/kr/ps/ext/dl/parser.cpp): a projection of a q_head onto its columns in their order, b_nonempty of a q_head
/// without columns, or n_value of one with one column.
template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
std::optional<std::string> read_head(urd::index_view<runir::kr::dl::QueryProjection<Category>, C> view)
{
    const auto columns = view.get_columns();
    const auto arity = std::same_as<Category, runir::kr::dl::ConceptTag> ? size_t { 1 } : size_t { 2 };
    auto result = query_head(view.get_arg(), arity);
    if (result)
        std::visit(
            [&](auto concrete)
            {
                if constexpr (requires { concrete.get_relation(); })
                    if (!std::ranges::equal(concrete.get_columns(), columns))
                        result.reset();
            },
            view.get_arg().get_variant());
    return result;
}
template<typename C>
std::optional<std::string> read_head(urd::index_view<runir::kr::dl::Boolean<runir::kr::dl::SemanticFamily, runir::kr::dl::NonemptyTag>, C> view)
{
    return std::visit([&](auto arg) { return query_head(arg, 0); }, view.get_arg());
}
template<typename C>
std::optional<std::string> read_head(urd::index_view<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::ValueTag>, C> view)
{ return query_head(view.get_arg(), 1); }
template<typename V>
std::optional<std::string> read_head(V)
{ return std::nullopt; }
}

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
void append_feature(std::ostream& os, urd::index_view<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>, C> view)
{
    os << ygg::print_indent << "(:" << feature_type<FeatureTag>() << "\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << "(:symbol " << view.get_symbol() << ")\n";
        // A feature that reads a :datalog head whole prints as it is written, so that the program parses again.
        auto head = std::optional<std::string>();
        if constexpr (requires { view.get_expression().get_variant(); })
            head = std::visit([](auto concrete) { return detail::read_head(concrete); }, view.get_expression().get_variant());
        if (head)
            os << ygg::print_indent << "(:head " << *head << ")\n";
        else
        {
            os << ygg::print_indent << "(:expression ";
            fmt::format_to(std::ostream_iterator<char>(os), "{}", view.get_expression());
            os << ")\n";
        }
    }
    os << ygg::print_indent << ')';
}

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
void append_feature(std::ostream& os, urd::index_view<runir::kr::ps::Feature<Family, FeatureTag>, C> view)
{
    std::visit([&](auto concrete) { append_feature(os, concrete); }, view.get_variant());
}

template<typename View>
std::string feature(View view)
{
    auto os = std::ostringstream {};
    append_feature(os, view);
    return os.str();
}

template<runir::kr::FamilyTag Family,
         typename Out,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         runir::kr::ps::dl::ConditionObservationTag<FeatureTag> ObservationTag,
         typename C>
Out condition(urd::index_view<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>, C> view, const Out& out)
{ return fmt::format_to(out, "({} {})", ObservationTag::keyword, view.get_feature().get_symbol()); }

template<typename Out, typename... Views>
Out condition(const std::variant<Views...>& view, Out out)
{
    return std::visit([&](auto concrete) { return condition(concrete, out); }, view);
}

template<runir::kr::FamilyTag Family,
         typename Out,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         runir::kr::ps::dl::EffectObservationTag<FeatureTag> ObservationTag,
         typename C>
Out effect(urd::index_view<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>, C> view, const Out& out)
{ return fmt::format_to(out, "({} {})", ObservationTag::keyword, view.get_feature().get_symbol()); }

template<typename Out, typename... Views>
Out effect(const std::variant<Views...>& view, Out out)
{
    return std::visit([&](auto concrete) { return effect(concrete, out); }, view);
}

template<typename View>
std::string condition(View view)
{
    auto result = std::string {};
    condition(view, std::back_inserter(result));
    return result;
}

template<typename View>
std::string effect(View view)
{
    auto result = std::string {};
    effect(view, std::back_inserter(result));
    return result;
}

// Programs (ext, icp): each family appends its own rule kinds, module header and program.

template<typename Values, typename Append>
void append_inline_section(std::ostream& os, std::string_view name, Values&& values, Append append)
{
    os << ygg::print_indent << "(:" << name;
    for (auto&& value : std::forward<Values>(values))
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

template<runir::kr::dl::CategoryTag Category, typename Arguments>
void append_declarations(std::ostream& os, Arguments arguments)
{
    for (auto argument : arguments)
        os << " (:" << Category::name << ' ' << argument.get_name() << ')';
}

template<ProgramFamilyTag Family, typename C>
void append_rule(std::ostream& os, urd::index_view<Rule<Family>, C> view)
{
    std::visit([&](auto rule) { append_rule(os, rule); }, view.get_variant());
}

/// A module's registers, memory, features and rules: what follows the family's module arguments.
template<typename Module>
void append_module_body(std::ostream& os, Module view)
{
    os << ygg::print_indent << "(:registers";
    append_declarations<runir::kr::dl::ConceptTag>(os, view.template get_registers<runir::kr::dl::ConceptTag>());
    append_declarations<runir::kr::dl::RoleTag>(os, view.template get_registers<runir::kr::dl::RoleTag>());
    os << ")\n";

    os << ygg::print_indent << "(:entry " << view.get_entry_memory_state().get_name() << ")\n";
    os << ygg::print_indent << "(:memory";
    for (auto state : view.get_memory_states())
        os << ' ' << state.get_name();
    os << ")\n";

    if constexpr (requires { view.get_datalog(); })
        if (!view.get_datalog().empty())
            os << ygg::print_indent << "(:datalog " << view.get_datalog() << ")\n";

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
        append_features(view.template get_features<runir::kr::dl::BooleanTag>());
        append_features(view.template get_features<runir::kr::dl::NumericalTag>());
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
                    std::visit(
                        [&](auto first_rule)
                        {
                            os << ygg::print_indent << "(:source-memory " << first_rule.get_source().get_name() << ")\n";
                            if constexpr (requires { first_rule.get_target(); })
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

template<typename View>
std::string rule(View view)
{
    auto os = std::ostringstream {};
    append_rule(os, view);
    return os.str();
}

template<typename View>
std::string module(View view)
{
    auto os = std::ostringstream {};
    append_module(os, view);
    return os.str();
}

template<typename View>
std::string program(View view)
{
    auto os = std::ostringstream {};
    append_program(os, view);
    return os.str();
}

}  // namespace runir::kr::ps

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::Feature<Family, FeatureTag>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::feature(view), context); }
};

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::feature(view), context); }
};

template<runir::kr::FamilyTag Family,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         runir::kr::ps::dl::ConditionObservationTag<FeatureTag> ObservationTag,
         typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>, C>> :
    fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::condition(view), context); }
};

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, runir::kr::ps::dl::EffectObservationTag<FeatureTag> ObservationTag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>, C>> :
    fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::effect(view), context); }
};

template<runir::kr::ps::ProgramFamilyTag Family, typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::MemoryState<Family>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return fmt::format_to(context.out(), "{}", view.get_name()); }
};

template<runir::kr::ps::ProgramFamilyTag Family, typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::ModuleSymbol<Family>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(auto view, format_context& context) const { return fmt::format_to(context.out(), "{}", view.get_name()); }
};

template<runir::kr::ps::ProgramFamilyTag Family, typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::Rule<Family>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::rule(view), context); }
};

template<runir::kr::ps::ProgramFamilyTag Family>
struct fmt::formatter<runir::kr::ps::ProgramProofEdgeLabel<Family>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& label, format_context& context) const
    {
        if (label.rule)
            return fmt::format_to(context.out(), "rule={}", label.rule->get_symbol());
        return fmt::format_to(context.out(), "rule=<none>");
    }
};

#endif
