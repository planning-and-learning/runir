#ifndef RUNIR_KR_PS_BASE_FORMATTER_HPP_
#define RUNIR_KR_PS_BASE_FORMATTER_HPP_

#include "runir/datasets/formatter.hpp"
#include "runir/formatter.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/dl/formatter.hpp"
#include "runir/kr/ps/base/sketch_proof_graph.hpp"
#include "runir/kr/ps/formatter.hpp"

#include <fmt/format.h>
#include <tyr/declarations.hpp>

namespace runir::kr::ps::base
{
template<tyr::TaskKind Kind>
struct SketchProofResults;
}  // namespace runir::kr::ps::base

// The base family as its parser reads it: each formatter's format is defined in src/kr/ps/base/formatter.cpp and
// explicitly instantiated there for the repositories that hold its records (its own, and ext's, which holds them too).

/// A rule: (:rule its symbol and (:expression its conditions and effects)), one section per line.
template<typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>, C>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>, C>& value, fmt::format_context& context) const;
};

/// A sketch: (:sketch its features and its rules), one per line, and a final newline.
template<typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::base::Sketch, C>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::ps::base::Sketch, C>& value, fmt::format_context& context) const;
};

/// A proof search's outcome: SketchProofResults(status=..., its statistics, its graph's size and its state sets' sizes).
template<tyr::TaskKind Kind>
struct fmt::formatter<runir::kr::ps::base::SketchProofResults<Kind>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::base::SketchProofResults<Kind>& value, fmt::format_context& context) const;
};

/// A proof graph's vertex: state=... initial=... goal=... alive=... unsolvable=...
template<tyr::TaskKind Kind>
struct fmt::formatter<runir::kr::ps::base::SketchProofVertexLabel<Kind>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::base::SketchProofVertexLabel<Kind>& value, fmt::format_context& context) const;
};

/// A proof graph's edge: rule=symbol action=...
template<>
struct fmt::formatter<runir::kr::ps::base::SketchProofEdgeLabel> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::base::SketchProofEdgeLabel& value, fmt::format_context& context) const;
};

#endif
