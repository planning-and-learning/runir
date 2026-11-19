#ifndef RUNIR_KR_UNS_FORMATTER_HPP_
#define RUNIR_KR_UNS_FORMATTER_HPP_

#include "runir/formatter.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <fmt/format.h>

// A classifier as its parser reads it: each view (through runir::kr::uns::Repository) has an fmt::formatter whose format
// is defined in src/kr/uns/formatter.cpp.

/// A classifier literal: its feature's symbol, or (not symbol).
template<>
struct fmt::formatter<runir::kr::uns::ClassifierLiteralView> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::uns::ClassifierLiteralView& value, fmt::format_context& context) const;
};

/// A classifier clause: (and literals...).
template<>
struct fmt::formatter<runir::kr::uns::ClassifierClauseView> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::uns::ClassifierClauseView& value, fmt::format_context& context) const;
};

/// A classifier: (:classifier (:symbol s) (:features ...) (:expression (or clauses...))), one section per line, indented by 4 spaces per level.
template<>
struct fmt::formatter<runir::kr::uns::ClassifierView> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::uns::ClassifierView& value, fmt::format_context& context) const;
};

#endif
