#ifndef RUNIR_GRAMMAR_FORMATTER_HPP_
#define RUNIR_GRAMMAR_FORMATTER_HPP_

#include "runir/formatter.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/formatter.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

#include <fmt/format.h>
#include <urd/elements/view.hpp>

// A grammar's records (through grammar::ConstructorRepository): format is defined in src/kr/dl/grammar/formatter.cpp,
// and explicitly instantiated there for each category. Its constructors are written by runir/kr/dl/formatter.hpp.

/// A non-terminal: its name.
template<runir::kr::dl::CategoryTag Category>
struct fmt::formatter<urd::index_view<runir::kr::dl::grammar::NonTerminal<Category>, runir::kr::dl::grammar::ConstructorRepository>> :
    runir::detail::without_format_spec
{
    fmt::format_context::iterator
    format(const urd::index_view<runir::kr::dl::grammar::NonTerminal<Category>, runir::kr::dl::grammar::ConstructorRepository>& value,
           fmt::format_context& context) const;
};

/// A derivation rule: (lhs (alternative or alternative ...)), a constructor alternative without its parentheses.
template<runir::kr::dl::CategoryTag Category>
struct fmt::formatter<urd::index_view<runir::kr::dl::grammar::DerivationRule<Category>, runir::kr::dl::grammar::ConstructorRepository>> :
    runir::detail::without_format_spec
{
    fmt::format_context::iterator
    format(const urd::index_view<runir::kr::dl::grammar::DerivationRule<Category>, runir::kr::dl::grammar::ConstructorRepository>& value,
           fmt::format_context& context) const;
};

/// A grammar: (, its derivation rules one per line indented by 4 (concept, role, boolean, numerical), ), each line
/// ending in a newline.
template<>
struct fmt::formatter<urd::index_view<runir::kr::dl::grammar::GrammarTag, runir::kr::dl::grammar::ConstructorRepository>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::dl::grammar::GrammarTag, runir::kr::dl::grammar::ConstructorRepository>& value,
                                         fmt::format_context& context) const;
};

#endif
