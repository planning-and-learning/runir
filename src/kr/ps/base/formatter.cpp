#include "runir/kr/ps/base/formatter.hpp"

#include "runir/kr/dl/semantics/formatter.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_executor_data.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/proof_status.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string_view>
#include <tyr/formalism/planning/formatter.hpp>
#include <urd/format.hpp>
#include <yggdrasil/formatting/cista_formatters.hpp>

// A section of conditions or effects is written on one line when it has at most one, (:effects) or (:effects e), and
// otherwise one per line.

namespace rk = runir::kr;
namespace rp = runir::kr::ps;
namespace rb = runir::kr::ps::base;

template<typename C>
fmt::format_context::iterator
fmt::formatter<urd::index_view<rp::Rule<rk::BaseFamilyTag>, C>>::format(const urd::index_view<rp::Rule<rk::BaseFamilyTag>, C>& value,
                                                                        fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(:rule");
    {
        const auto deeper = urd::scoped_indent();
        out = fmt::format_to(out, "{}(:symbol {})", urd::new_line(), std::string_view(value.get_symbol()));
        out = fmt::format_to(out, "{}(:expression", urd::new_line());
        {
            const auto expression = urd::scoped_indent();
            const auto write_section = [&](std::string_view name, auto values)
            {
                if (values.empty())
                    out = fmt::format_to(out, "{}(:{})", urd::new_line(), name);
                else if (values.size() == 1)
                    out = fmt::format_to(out, "{}(:{} {})", urd::new_line(), name, values.front());
                else
                {
                    out = fmt::format_to(out, "{}(:{}", urd::new_line(), name);
                    {
                        const auto items = urd::scoped_indent();
                        for (auto item : values)
                            out = fmt::format_to(out, "{}{}", urd::new_line(), item);
                    }
                    out = fmt::format_to(out, "{})", urd::new_line());
                }
            };
            write_section("conditions", value.get_conditions());
            write_section("effects", value.get_effects());
        }
        out = fmt::format_to(out, "{})", urd::new_line());
    }
    return fmt::format_to(out, "{})", urd::new_line());
}

template<typename C>
fmt::format_context::iterator fmt::formatter<urd::index_view<rb::Sketch, C>>::format(const urd::index_view<rb::Sketch, C>& value,
                                                                                     fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "(:sketch");
    {
        const auto deeper = urd::scoped_indent();
        out = fmt::format_to(out, "{}(:features", urd::new_line());
        {
            const auto features = urd::scoped_indent();
            for (auto feature : value.template get_features<rk::dl::BooleanTag>())
                out = fmt::format_to(out, "{}{}", urd::new_line(), feature);
            for (auto feature : value.template get_features<rk::dl::NumericalTag>())
                out = fmt::format_to(out, "{}{}", urd::new_line(), feature);
        }
        out = fmt::format_to(out, "{})", urd::new_line());
        out = fmt::format_to(out, "{}(:rules", urd::new_line());
        {
            const auto rules = urd::scoped_indent();
            for (auto rule : value.get_rules())
                out = fmt::format_to(out, "{}{}", urd::new_line(), rule);
        }
        out = fmt::format_to(out, "{})", urd::new_line());
    }
    // A sketch ends its last line.
    return fmt::format_to(out, "{})\n", urd::new_line());
}

template<tyr::TaskKind Kind>
fmt::format_context::iterator fmt::formatter<rb::SketchProofResults<Kind>>::format(const rb::SketchProofResults<Kind>& value,
                                                                                   fmt::format_context& context) const
{
    return fmt::format_to(context.out(),
                          "SketchProofResults(status={}, num_expanded={}, num_generated={}, graph_vertices={}, graph_edges={}, "
                          "deadend_states={}, open_states={}, cycle={})",
                          rp::to_string(value.status),
                          value.statistics.num_expanded,
                          value.statistics.num_generated,
                          value.graph ? value.graph->num_vertices() : 0,
                          value.graph ? value.graph->num_edges() : 0,
                          value.deadend_states.size(),
                          value.open_states.size(),
                          value.cycle.size());
}

template<tyr::TaskKind Kind>
fmt::format_context::iterator fmt::formatter<rb::SketchProofVertexLabel<Kind>>::format(const rb::SketchProofVertexLabel<Kind>& value,
                                                                                       fmt::format_context& context) const
{
    return fmt::format_to(context.out(),
                          "state={} initial={} goal={} alive={} unsolvable={}",
                          value.state.index(),
                          value.is_initial,
                          value.is_goal,
                          value.is_alive,
                          value.is_unsolvable);
}

fmt::format_context::iterator fmt::formatter<rb::SketchProofEdgeLabel>::format(const rb::SketchProofEdgeLabel& value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "rule={} action={}", value.rule.get_symbol(), value.action); }

template struct fmt::formatter<urd::index_view<rp::Rule<rk::BaseFamilyTag>, rb::Repository>>;
template struct fmt::formatter<urd::index_view<rp::Rule<rk::BaseFamilyTag>, rp::ext::Repository>>;
template struct fmt::formatter<urd::index_view<rb::Sketch, rb::Repository>>;
template struct fmt::formatter<urd::index_view<rb::Sketch, rp::ext::Repository>>;
template struct fmt::formatter<rb::SketchProofResults<tyr::GroundTag>>;
template struct fmt::formatter<rb::SketchProofResults<tyr::LiftedTag>>;
template struct fmt::formatter<rb::SketchProofVertexLabel<tyr::GroundTag>>;
template struct fmt::formatter<rb::SketchProofVertexLabel<tyr::LiftedTag>>;
