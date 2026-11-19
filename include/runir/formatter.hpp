#ifndef RUNIR_FORMATTER_HPP_
#define RUNIR_FORMATTER_HPP_

#include <fmt/format.h>

// runir's records are written by fmt::formatter specializations of their views, declared in each module's formatter.hpp
// and defined in the library: written once there, not in every file that prints a record. A record over several lines
// begins each line after its first with urd::new_line() and nests its sections under urd::scoped_indent, so that a
// record written inside another is indented with it.

namespace runir::detail
{
/// The parse of runir's formatters: a record takes no format spec.
struct without_format_spec
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& context) const { return context.begin(); }
};
}  // namespace runir::detail

#endif
