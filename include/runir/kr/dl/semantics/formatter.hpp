#ifndef RUNIR_SEMANTICS_FORMATTER_HPP_
#define RUNIR_SEMANTICS_FORMATTER_HPP_

#include "runir/formatter.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/formatter.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>  // fmt::range_format_kind
#include <type_traits>
#include <urd/elements/flat.hpp>
#include <urd/elements/view.hpp>

// The semantic records: constructors, queries and their references through dl::ConstructorRepository, denotations,
// registers and call arguments through semantics::DenotationRepository. Each format is defined in
// src/kr/dl/semantics/formatter.cpp, and explicitly instantiated there for each category or query tag.

/// A denotation is written by its formatter, not as a range of its elements.
template<runir::kr::dl::CategoryTag Category, typename Char>
struct fmt::range_format_kind<urd::index_view<runir::kr::dl::semantics::Denotation<Category>, runir::kr::dl::semantics::DenotationRepository>, Char, void> :
    std::false_type
{
};

/// A query column: its name.
template<>
struct fmt::formatter<urd::index_view<runir::kr::dl::QueryColumn, runir::kr::dl::ConstructorRepository>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::dl::QueryColumn, runir::kr::dl::ConstructorRepository>& value,
                                         fmt::format_context& context) const;
};

/// A query: (keyword components...); a query of any kind (Query<>) as the query it holds.
template<typename Tag>
struct fmt::formatter<urd::index_view<runir::kr::dl::Query<Tag>, runir::kr::dl::ConstructorRepository>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::dl::Query<Tag>, runir::kr::dl::ConstructorRepository>& value,
                                         fmt::format_context& context) const;
};

/// A query projection: (c_project column query) or (r_project column column query).
template<runir::kr::dl::ConceptOrRoleTag Category>
struct fmt::formatter<urd::index_view<runir::kr::dl::QueryProjection<Category>, runir::kr::dl::ConstructorRepository>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::dl::QueryProjection<Category>, runir::kr::dl::ConstructorRepository>& value,
                                         fmt::format_context& context) const;
};

/// An argument: its name.
template<runir::kr::dl::CategoryTag Category>
struct fmt::formatter<urd::index_view<runir::kr::dl::Argument<Category>, runir::kr::dl::ConstructorRepository>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::dl::Argument<Category>, runir::kr::dl::ConstructorRepository>& value,
                                         fmt::format_context& context) const;
};

/// A register: its name.
template<runir::kr::dl::ConceptOrRoleTag Category>
struct fmt::formatter<urd::index_view<runir::kr::dl::Register<Category>, runir::kr::dl::ConstructorRepository>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::dl::Register<Category>, runir::kr::dl::ConstructorRepository>& value,
                                         fmt::format_context& context) const;
};

/// A constructor of a category: the constructor record it holds.
template<runir::kr::dl::CategoryTag Category>
struct fmt::formatter<urd::index_view<runir::kr::dl::Constructor<Category>, runir::kr::dl::ConstructorRepository>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::dl::Constructor<Category>, runir::kr::dl::ConstructorRepository>& value,
                                         fmt::format_context& context) const;
};

/// A denotation: true or false, a number, concept<bits>, or role{(source, target), ...}.
template<runir::kr::dl::CategoryTag Category>
struct fmt::formatter<urd::index_view<runir::kr::dl::semantics::Denotation<Category>, runir::kr::dl::semantics::DenotationRepository>> :
    runir::detail::without_format_spec
{
    fmt::format_context::iterator
    format(const urd::index_view<runir::kr::dl::semantics::Denotation<Category>, runir::kr::dl::semantics::DenotationRepository>& value,
           fmt::format_context& context) const;
};

/// A concept's or role's values as its record holds them: its words ([5, 0]).
template<runir::kr::dl::ConceptOrRoleTag Category>
struct fmt::formatter<runir::kr::dl::semantics::DenotationWords<Category>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::dl::semantics::DenotationWords<Category>& value, fmt::format_context& context) const;
};

#endif
