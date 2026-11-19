#ifndef RUNIR_CNF_GRAMMAR_FORMATTER_HPP_
#define RUNIR_CNF_GRAMMAR_FORMATTER_HPP_

#include "runir/formatter.hpp"
#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/formatter.hpp"

#include <fmt/format.h>
#include <urd/elements/view.hpp>

// A CNF grammar's records (through cnf_grammar::ConstructorRepository): format is defined in
// src/kr/dl/cnf_grammar/formatter.cpp, and explicitly instantiated there for each category. Its constructors are
// written by runir/kr/dl/formatter.hpp.

/// A non-terminal: its name.
template<runir::kr::dl::CategoryTag Category>
struct fmt::formatter<urd::index_view<runir::kr::dl::cnf_grammar::NonTerminal<Category>, runir::kr::dl::cnf_grammar::ConstructorRepository>> :
    runir::detail::without_format_spec
{
    fmt::format_context::iterator
    format(const urd::index_view<runir::kr::dl::cnf_grammar::NonTerminal<Category>, runir::kr::dl::cnf_grammar::ConstructorRepository>& value,
           fmt::format_context& context) const;
};

/// A derivation rule: (lhs (constructor)), the constructor without its parentheses.
template<runir::kr::dl::CategoryTag Category>
struct fmt::formatter<urd::index_view<runir::kr::dl::cnf_grammar::DerivationRule<Category>, runir::kr::dl::cnf_grammar::ConstructorRepository>> :
    runir::detail::without_format_spec
{
    fmt::format_context::iterator
    format(const urd::index_view<runir::kr::dl::cnf_grammar::DerivationRule<Category>, runir::kr::dl::cnf_grammar::ConstructorRepository>& value,
           fmt::format_context& context) const;
};

/// A substitution rule: (lhs (rhs)), both non-terminals.
template<runir::kr::dl::CategoryTag Category>
struct fmt::formatter<urd::index_view<runir::kr::dl::cnf_grammar::SubstitutionRule<Category>, runir::kr::dl::cnf_grammar::ConstructorRepository>> :
    runir::detail::without_format_spec
{
    fmt::format_context::iterator
    format(const urd::index_view<runir::kr::dl::cnf_grammar::SubstitutionRule<Category>, runir::kr::dl::cnf_grammar::ConstructorRepository>& value,
           fmt::format_context& context) const;
};

/// A CNF grammar: (, its derivation rules then its substitution rules one per line indented by 4 (each by concept,
/// role, boolean, numerical), ), each line ending in a newline.
template<>
struct fmt::formatter<urd::index_view<runir::kr::dl::cnf_grammar::Grammar, runir::kr::dl::cnf_grammar::ConstructorRepository>> :
    runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::dl::cnf_grammar::Grammar, runir::kr::dl::cnf_grammar::ConstructorRepository>& value,
                                         fmt::format_context& context) const;
};

#endif
