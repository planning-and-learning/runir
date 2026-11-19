#ifndef RUNIR_KR_PS_EXT_FORMATTER_HPP_
#define RUNIR_KR_PS_EXT_FORMATTER_HPP_

#include "runir/formatter.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/dl/formatter.hpp"
#include "runir/kr/ps/formatter.hpp"

#include <fmt/format.h>
#include <tyr/declarations.hpp>

namespace runir::kr::ps::ext
{
template<tyr::TaskKind Kind>
struct ProgramProofResults;
}  // namespace runir::kr::ps::ext

// The ext family as its parser reads it: each formatter's format is defined in src/kr/ps/ext/formatter.cpp and
// explicitly instantiated there for each rule kind and task kind. A rule, a module and a program are written one section
// per line, nested sections indented by four spaces.

/// An order direction: min or max.
template<>
struct fmt::formatter<runir::kr::ps::ext::OrderDirection> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(runir::kr::ps::ext::OrderDirection value, fmt::format_context& context) const;
};

/// A rule: (:kind its conditions, its kind's sections, its effects), one per line.
template<runir::kr::ps::ext::RuleKind Kind>
struct fmt::formatter<runir::kr::ps::ext::RuleView<Kind>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::ext::RuleView<Kind>& value, fmt::format_context& context) const;
};

/// A rule of any kind: as its concrete rule.
template<>
struct fmt::formatter<runir::kr::ps::ext::RuleVariantView> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::ext::RuleVariantView& value, fmt::format_context& context) const;
};

/// A module: (:module its symbol, arguments, registers, entry, memory, features and rules), one per line.
template<>
struct fmt::formatter<runir::kr::ps::ext::ModuleView> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::ext::ModuleView& value, fmt::format_context& context) const;
};

/// A program: (:program its entry and its modules), one per line.
template<>
struct fmt::formatter<runir::kr::ps::ext::ProgramView> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::ext::ProgramView& value, fmt::format_context& context) const;
};

/// A proof graph's vertex: state=... module=... initial=... goal=... alive=... unsolvable=...
template<tyr::TaskKind Kind>
struct fmt::formatter<runir::kr::ps::ProgramProofVertexLabel<runir::kr::ExtFamilyTag, Kind>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::ProgramProofVertexLabel<runir::kr::ExtFamilyTag, Kind>& value,
                                         fmt::format_context& context) const;
};

/// A proof search's outcome: ProgramProofResults(status=..., its graph's size, its state sets' sizes and statistics).
template<tyr::TaskKind Kind>
struct fmt::formatter<runir::kr::ps::ext::ProgramProofResults<Kind>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::ext::ProgramProofResults<Kind>& value, fmt::format_context& context) const;
};

#endif
