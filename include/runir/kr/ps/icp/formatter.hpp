#ifndef RUNIR_KR_PS_ICP_FORMATTER_HPP_
#define RUNIR_KR_PS_ICP_FORMATTER_HPP_

#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/formatter.hpp"
#include "runir/kr/ps/icp/execution_view.hpp"
#include "runir/kr/ps/icp/module_view.hpp"
#include "runir/kr/ps/icp/program_view.hpp"
#include "runir/kr/ps/icp/repository.hpp"
#include "runir/kr/ps/icp/rule_view.hpp"
#include "runir/kr/ps/icp/xcondition_view.hpp"
#include "runir/kr/ps/icp/xeffect_view.hpp"
#include "runir/kr/ps/memory_state_view.hpp"
#include "runir/kr/ps/module_symbol_view.hpp"
#include "runir/kr/ps/program_proof_graph.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <tyr/formalism/planning/formatter.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <variant>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps::icp
{

/// An operation, a position and a reset pair as fmt writes them (format_as).
inline const char* format_as(ConditionOperation operation) noexcept { return operation == ConditionOperation::BELONGS ? "belongs" : "not-belongs"; }

inline const char* format_as(EffectOperation operation) noexcept { return operation == EffectOperation::ENTER ? "enter" : "exit"; }

inline ygg::uint_t format_as(ArgumentPosition position) noexcept { return position.value; }

inline std::string format_as(const ResetPair& pair) { return fmt::format("({}, {})", pair.before, pair.after); }

template<typename Predicate, typename ArgumentNames>
void append_indexical(std::ostream& os, Predicate predicate, const ArgumentNames& arguments)
{
    os << '(' << format_as(predicate.get_operation());
    std::visit(
        [&]<typename Reference>(const Reference& reference)
        {
            if constexpr (std::same_as<Reference, ArgumentPosition>)
            {
                os << " (:argument ";
                const auto position = reference.value;
                if (position < arguments.size())
                    os << arguments.at(position);
                else
                    os << "arg" << position;
                os << ')';
            }
            else
                os << " (:register (:concept " << reference.get_name() << "))";
        },
        predicate.get_object());
    os << " (:concept " << std::visit([](auto concrete) { return concrete.get_symbol(); }, predicate.get_feature().get_variant()) << "))";
}

template<runir::kr::ps::icp::RuleKind Kind, typename C>
void append_rule_body(std::ostream& os, urd::index_view<runir::kr::ps::icp::Rule<Kind>, C> view)
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
        using Category = RuleCategoryFor<Kind>;
        os << ygg::print_indent << "(:" << Category::keyword << ' '
           << std::visit([](auto concrete) { return concrete.get_symbol(); }, view.get_feature().get_variant()) << ")\n";
        os << ygg::print_indent << "(:register (:" << Category::keyword << ' ' << view.get_slot().get_name() << "))\n";
        append_effects(os, view.get_effects());
    }
}

template<runir::kr::ps::icp::RuleKind Kind, typename C>
void append_rule(std::ostream& os, urd::index_view<runir::kr::ps::icp::Rule<Kind>, C> view)
{
    os << ygg::print_indent << "(:" << Kind::keyword << "\n";
    {
        ygg::IndentScope scope(os);
        append_rule_body(os, view);
    }
    os << ygg::print_indent << ')';
}

template<typename C>
void append_module(std::ostream& os, urd::index_view<runir::kr::ps::icp::Module, C> view)
{
    os << ygg::print_indent << "(:module\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << "(:symbol " << view.get_symbol().get_name() << ")\n";

        os << ygg::print_indent << "(:arguments)\n";

        append_module_body(os, view);
        os << ygg::print_indent << "(:reset-spo";
        for (const auto& pair : view.get_reset_pairs())
            os << " (:prec " << std::visit([](auto concrete) { return concrete.get_symbol(); }, urd::resolve(pair.before, view.context()).get_variant()) << ' '
               << std::visit([](auto concrete) { return concrete.get_symbol(); }, urd::resolve(pair.after, view.context()).get_variant()) << ')';
        os << ")\n";
    }
    os << ygg::print_indent << ')';
}

template<typename C>
void append_program(std::ostream& os, urd::index_view<runir::kr::ps::icp::Program, C> view)
{
    os << ygg::print_indent << "(:program\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << "(:entry " << view.get_entry_mod().get_symbol().get_name() << ")\n";
        append_module(os, view.get_entry_mod());
        os << '\n';
    }
    os << ygg::print_indent << ')';
}

}  // namespace runir::kr::ps::icp

template<runir::kr::ps::icp::RuleKind Kind, typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::icp::Rule<Kind>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::rule(view), context); }
};

template<typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::icp::Module, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::module(view), context); }
};

template<typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::icp::Program, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::program(view), context); }
};

template<typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::icp::XCondition, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const
    {
        std::ostringstream os;
        runir::kr::ps::icp::append_indexical(os, view, urd::Store<urd::sequence<std::string>> {});
        return fmt::formatter<std::string_view>::format(os.str(), context);
    }
};

template<typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::icp::XEffect, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const
    {
        std::ostringstream os;
        runir::kr::ps::icp::append_indexical(os, view, urd::Store<urd::sequence<std::string>> {});
        return fmt::formatter<std::string_view>::format(os.str(), context);
    }
};

template<typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::icp::Histories, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }

    auto format(const auto& value, format_context& context) const { return fmt::format_to(context.out(), "Histories(concepts={})", value.get_concepts()); }
};

template<tyr::TaskKind Kind, typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::icp::ProgramState<Kind>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }

    auto format(const auto& value, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "ProgramState(program={}, memory_state={}, registers={}, histories={}, state={})",
                              value.get_program().index(),
                              value.get_memory_state(),
                              value.get_registers(),
                              value.get_histories(),
                              value.get_state().index());
    }
};

template<tyr::TaskKind Kind>
struct fmt::formatter<runir::kr::ps::ProgramProofVertexLabel<runir::kr::IcpFamilyTag, Kind>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }

    auto format(const auto& label, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "state={} initial={} goal={} alive={} unsolvable={}",
                              label.program_state.get_state().index(),
                              label.is_initial,
                              label.is_goal,
                              label.is_alive,
                              label.is_unsolvable);
    }
};

#endif
