#ifndef RUNIR_KR_DL_FORMATTER_HPP_
#define RUNIR_KR_DL_FORMATTER_HPP_

#include "runir/formatter.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <fmt/format.h>
#include <urd/elements/view.hpp>
#include <utility>

// The constructor records of each family: each record's view has an fmt::formatter whose format is defined in
// src/kr/dl/formatter.cpp, and explicitly instantiated there for each family F with the repository C its records are
// viewed through (SemanticFamily with dl::ConstructorRepository, grammar::GrammarFamily with
// grammar::ConstructorRepository, cnf_grammar::CnfGrammarFamily with cnf_grammar::ConstructorRepository) and each of
// F's constructor tags.

namespace runir::kr::dl
{
/// Selects a constructor's text without its parentheses, keyword components..., as a grammar rule writes an alternative:
/// std::pair { view, Unparenthesized {} }.
struct Unparenthesized
{
};
}  // namespace runir::kr::dl

/// A concept constructor: (keyword components...), its identifying members in record order.
template<typename F, typename Tag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::Concept<F, Tag>, C>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::dl::Concept<F, Tag>, C>& value, fmt::format_context& context) const;
};

/// A concept constructor without its parentheses: keyword components...
template<typename F, typename Tag, typename C>
struct fmt::formatter<std::pair<urd::index_view<runir::kr::dl::Concept<F, Tag>, C>, runir::kr::dl::Unparenthesized>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const std::pair<urd::index_view<runir::kr::dl::Concept<F, Tag>, C>, runir::kr::dl::Unparenthesized>& value,
                                         fmt::format_context& context) const;
};

/// A role constructor: (keyword components...), its identifying members in record order.
template<typename F, typename Tag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::Role<F, Tag>, C>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::dl::Role<F, Tag>, C>& value, fmt::format_context& context) const;
};

/// A role constructor without its parentheses: keyword components...
template<typename F, typename Tag, typename C>
struct fmt::formatter<std::pair<urd::index_view<runir::kr::dl::Role<F, Tag>, C>, runir::kr::dl::Unparenthesized>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const std::pair<urd::index_view<runir::kr::dl::Role<F, Tag>, C>, runir::kr::dl::Unparenthesized>& value,
                                         fmt::format_context& context) const;
};

/// A boolean constructor: (keyword components...), its identifying members in record order.
template<typename F, typename Tag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::Boolean<F, Tag>, C>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::dl::Boolean<F, Tag>, C>& value, fmt::format_context& context) const;
};

/// A boolean constructor without its parentheses: keyword components...
template<typename F, typename Tag, typename C>
struct fmt::formatter<std::pair<urd::index_view<runir::kr::dl::Boolean<F, Tag>, C>, runir::kr::dl::Unparenthesized>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const std::pair<urd::index_view<runir::kr::dl::Boolean<F, Tag>, C>, runir::kr::dl::Unparenthesized>& value,
                                         fmt::format_context& context) const;
};

/// A numerical constructor: (keyword components...), its identifying members in record order.
template<typename F, typename Tag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::Numerical<F, Tag>, C>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::dl::Numerical<F, Tag>, C>& value, fmt::format_context& context) const;
};

/// A numerical constructor without its parentheses: keyword components...
template<typename F, typename Tag, typename C>
struct fmt::formatter<std::pair<urd::index_view<runir::kr::dl::Numerical<F, Tag>, C>, runir::kr::dl::Unparenthesized>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const std::pair<urd::index_view<runir::kr::dl::Numerical<F, Tag>, C>, runir::kr::dl::Unparenthesized>& value,
                                         fmt::format_context& context) const;
};

#endif
