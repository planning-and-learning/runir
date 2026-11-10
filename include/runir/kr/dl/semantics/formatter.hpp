#ifndef RUNIR_SEMANTICS_FORMATTER_HPP_
#define RUNIR_SEMANTICS_FORMATTER_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/formatter.hpp"
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"

#include <cstddef>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <sstream>
#include <string>
#include <string_view>
#include <tyr/formalism/planning/formatter.hpp>
#include <utility>
#include <vector>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formatting/cista_formatters.hpp>
#include <urd/format.hpp>
#include <yggdrasil/formatting/formatter.hpp>

namespace runir::kr::dl::semantics
{

template<typename... Components>
std::string constructor(std::string_view keyword, Components&&... components)
{
    auto os = std::ostringstream {};
    os << '(' << keyword;
    ((os << ' ' << fmt::format("{}", std::forward<Components>(components))), ...);
    os << ')';
    return os.str();
}

template<typename Columns>
std::string query_columns(Columns columns)
{ return fmt::format("({})", fmt::join(columns, " ")); }

template<typename Tag, typename C>
std::string query(urd::index_view<runir::kr::dl::Query<Tag>, C> view)
{
    if constexpr (std::same_as<Tag, void>)
        return fmt::format("{}", view.get_variant());
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor("q_atomic_state", fmt::format("{:?}", view.get_predicate().get_name().view()), query_columns(view.get_columns()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor("q_atomic_goal",
                           fmt::format("{:?}", view.get_predicate().get_name().view()),
                           boolean(view.get_polarity()),
                           query_columns(view.get_columns()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QueryConceptTag>)
        return constructor(runir::kr::dl::QueryConceptTag::keyword, *view.get_columns().begin(), view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QueryRoleTag>)
        return constructor(runir::kr::dl::QueryRoleTag::keyword, query_columns(view.get_columns()), view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QueryProjectTag>)
        return constructor(runir::kr::dl::QueryProjectTag::keyword, query_columns(view.get_columns()), view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QueryRenameTag>)
        return constructor(runir::kr::dl::QueryRenameTag::keyword, query_columns(view.get_columns()), view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QuerySelectEqualTag>)
        return constructor(runir::kr::dl::QuerySelectEqualTag::keyword, view.get_lhs_column(), view.get_rhs_column(), view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QuerySelectValueTag>)
        return constructor(runir::kr::dl::QuerySelectValueTag::keyword,
                           view.get_column(),
                           fmt::format("{:?}", view.get_object().get_name().view()),
                           view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QueryJoinTag>)
        return constructor(runir::kr::dl::QueryJoinTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QueryUnionTag>)
        return constructor(runir::kr::dl::QueryUnionTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QueryDifferenceTag>)
        return constructor(runir::kr::dl::QueryDifferenceTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QueryGenericJoinTag>)
        return constructor(runir::kr::dl::QueryGenericJoinTag::keyword,
                           query_columns(view.get_variable_order()),
                           query_columns(view.get_output_order()),
                           fmt::format("{}", fmt::join(view.get_args(), " ")));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QueryEmptyTag>)
        return constructor(runir::kr::dl::QueryEmptyTag::keyword, query_columns(view.get_columns()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QueryMustPassTag>)
        return constructor(runir::kr::dl::QueryMustPassTag::keyword,
                           query_columns(view.get_keys()),
                           view.get_sources(),
                           view.get_edges(),
                           view.get_targets(),
                           view.get_probes());
    else
        static_assert(false, "unhandled DL query constructor tag");
}

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
std::string query_projection(urd::index_view<runir::kr::dl::QueryProjection<Category>, C> view)
{
    auto columns = view.get_columns();
    auto it = columns.begin();
    auto lhs = *it++;
    if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
        return constructor(runir::kr::dl::ConceptProjectSyntaxTag::keyword, lhs, view.get_arg());
    else
        return constructor(runir::kr::dl::RoleProjectSyntaxTag::keyword, lhs, *it, view.get_arg());
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string denotation(urd::index_view<runir::kr::dl::semantics::Denotation<Category>, C> view)
{
    if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
    {
        return boolean(view.value());
    }
    else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
    {
        return fmt::format("{}", view.value());
    }
    else if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        return fmt::format("{}{}", Category::name, view.value());
    }
    else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        auto pairs = urd::Store<urd::sequence<std::string>> {};
        for (auto [source, target] : view)
            pairs.push_back(fmt::format("({}, {})", source.index(), target.index()));
        return fmt::format("{}{{{}}}", Category::name, fmt::join(pairs, ", "));
    }
    else
    {
        static_assert(false, "unhandled DL denotation category in denotation");
    }
}

}  // namespace runir::kr::dl::semantics

template<runir::kr::dl::CategoryTag Category, typename C, typename Char>
struct fmt::range_format_kind<urd::index_view<runir::kr::dl::semantics::Denotation<Category>, C>, Char, void> : std::false_type
{
};

template<typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::QueryColumn, C>> : fmt::formatter<std::string_view>
{
    auto format(const urd::index_view<runir::kr::dl::QueryColumn, C>& value, format_context& ctx) const
    { return fmt::format_to(ctx.out(), "{}", value.get_name()); }
};

template<typename Tag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::Query<Tag>, C>> : fmt::formatter<std::string_view>
{
    using View = urd::index_view<runir::kr::dl::Query<Tag>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::query(view), ctx); }
};

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::QueryProjection<Category>, C>> : fmt::formatter<std::string_view>
{
    using View = urd::index_view<runir::kr::dl::QueryProjection<Category>, C>;
    auto format(View view, format_context& ctx) const
    { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::query_projection(view), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::Argument<Category>, C>> : fmt::formatter<std::string_view>
{
    auto format(const urd::index_view<runir::kr::dl::Argument<Category>, C>& value, format_context& ctx) const
    { return fmt::format_to(ctx.out(), "{}", value.get_name()); }
};

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::Register<Category>, C>> : fmt::formatter<std::string_view>
{
    auto format(const urd::index_view<runir::kr::dl::Register<Category>, C>& value, format_context& ctx) const
    { return fmt::format_to(ctx.out(), "{}", value.get_name()); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::Constructor<Category>, C>> : fmt::formatter<std::string_view>
{
    using View = urd::index_view<runir::kr::dl::Constructor<Category>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(fmt::format("{}", view.get_variant()), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::semantics::Denotation<Category>, C>> : fmt::formatter<std::string_view>
{
    using View = urd::index_view<runir::kr::dl::semantics::Denotation<Category>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::denotation(view), ctx); }
};

template<typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::semantics::RegisterValues, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& value, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "RegisterValues(concepts=[{}], roles=[{}])",
                              fmt::join(value.template values<runir::kr::dl::ConceptTag>(), ", "),
                              fmt::join(value.template values<runir::kr::dl::RoleTag>(), ", "));
    }
};

template<typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::semantics::CallArguments, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& value, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "CallArguments(concepts={}, roles={}, booleans={}, numericals={})",
                              value.template arguments<runir::kr::dl::ConceptTag>(),
                              value.template arguments<runir::kr::dl::RoleTag>(),
                              value.template arguments<runir::kr::dl::BooleanTag>(),
                              value.template arguments<runir::kr::dl::NumericalTag>());
    }
};

#endif
