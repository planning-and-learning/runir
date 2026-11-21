#include "runir/kr/dl/semantics/formatter.hpp"

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <runir/kr/database/syntax/formatter.hpp>
#include <string_view>
#include <tyr/formalism/planning/formatter.hpp>
#include <urd/format.hpp>
#include <yggdrasil/formatting/cista_formatters.hpp>

namespace dl = runir::kr::dl;
namespace ds = runir::kr::dl::semantics;

fmt::format_context::iterator
fmt::formatter<urd::index_view<dl::QueryColumn, dl::ConstructorRepository>>::format(const urd::index_view<dl::QueryColumn, dl::ConstructorRepository>& value,
                                                                                    fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value.get_name()); }

template<typename Tag>
fmt::format_context::iterator
fmt::formatter<urd::index_view<dl::Query<Tag>, dl::ConstructorRepository>>::format(const urd::index_view<dl::Query<Tag>, dl::ConstructorRepository>& value,
                                                                                   fmt::format_context& context) const
{
    if constexpr (std::same_as<Tag, void>)
        return fmt::format_to(context.out(), "{}", value.get_variant());
    else
    {
        // (keyword components...), each component after a space; a list of columns as (column column ...).
        auto out = context.out();
        const auto write = [&out](std::string_view keyword, const auto&... components)
        {
            out = fmt::format_to(out, "({}", keyword);
            ((out = fmt::format_to(out, " {}", components)), ...);
            out = fmt::format_to(out, ")");
            return out;
        };
        const auto columns = [](auto range) { return fmt::format("({})", fmt::join(range, " ")); };

        if constexpr (dl::is_atomic_state_tag_v<Tag>)
            return write("q_atomic_state", fmt::format("{:?}", value.get_predicate().get_name().view()), columns(value.get_columns()));
        else if constexpr (dl::is_atomic_goal_tag_v<Tag>)
            return write("q_atomic_goal",
                         fmt::format("{:?}", value.get_predicate().get_name().view()),
                         value.get_polarity() ? dl::TrueTag::keyword : dl::FalseTag::keyword,
                         columns(value.get_columns()));
        else if constexpr (std::same_as<Tag, dl::QueryConceptTag>)
            return write(dl::QueryConceptTag::keyword, *value.get_columns().begin(), value.get_operand());
        else if constexpr (std::same_as<Tag, dl::QueryRoleTag>)
            return write(dl::QueryRoleTag::keyword, columns(value.get_columns()), value.get_operand());
        else if constexpr (std::same_as<Tag, dl::QueryProjectTag>)
            return write(dl::QueryProjectTag::keyword, columns(value.get_columns()), value.get_operand());
        else if constexpr (std::same_as<Tag, dl::QueryRenameTag>)
            return write(dl::QueryRenameTag::keyword, columns(value.get_columns()), value.get_operand());
        else if constexpr (std::same_as<Tag, dl::QuerySelectEqualTag>)
            return write(dl::QuerySelectEqualTag::keyword, value.get_left_column(), value.get_right_column(), value.get_operand());
        else if constexpr (std::same_as<Tag, dl::QuerySelectValueTag>)
            return write(dl::QuerySelectValueTag::keyword, value.get_column(), fmt::format("{:?}", value.get_object().get_name().view()), value.get_operand());
        else if constexpr (std::same_as<Tag, dl::QueryJoinTag>)
            return write(dl::QueryJoinTag::keyword, value.get_left_operand(), value.get_right_operand());
        else if constexpr (std::same_as<Tag, dl::QueryUnionTag>)
            return write(dl::QueryUnionTag::keyword, value.get_left_operand(), value.get_right_operand());
        else if constexpr (std::same_as<Tag, dl::QueryDifferenceTag>)
            return write(dl::QueryDifferenceTag::keyword, value.get_left_operand(), value.get_right_operand());
        else if constexpr (std::same_as<Tag, dl::QueryGenericJoinTag>)
            return write(dl::QueryGenericJoinTag::keyword,
                         columns(value.get_variable_order()),
                         columns(value.get_output_order()),
                         fmt::format("{}", fmt::join(value.get_args(), " ")));
        else if constexpr (std::same_as<Tag, dl::QueryEmptyTag>)
            return write(dl::QueryEmptyTag::keyword, columns(value.get_columns()));
        else if constexpr (std::same_as<Tag, dl::QueryBooleanTag>)
            return write(dl::QueryBooleanTag::keyword, value.get_operand());
        else if constexpr (std::same_as<Tag, dl::QueryNumericalTag>)
            return write(dl::QueryNumericalTag::keyword, *value.get_columns().begin(), value.get_operand());
        else if constexpr (std::same_as<Tag, dl::QueryHeadTag>)
            return write(dl::QueryHeadTag::keyword, value.get_relation().get_name(), columns(value.get_columns()));
        else
            static_assert(false, "unhandled DL query constructor tag");
    }
}

template<dl::ConceptOrRoleTag Category>
fmt::format_context::iterator fmt::formatter<urd::index_view<dl::QueryProjection<Category>, dl::ConstructorRepository>>::format(
    const urd::index_view<dl::QueryProjection<Category>, dl::ConstructorRepository>& value,
    fmt::format_context& context) const
{
    auto columns = value.get_columns();
    auto it = columns.begin();
    auto lhs = *it++;
    if constexpr (std::same_as<Category, dl::ConceptTag>)
        return fmt::format_to(context.out(), "({} {} {})", dl::ConceptProjectSyntaxTag::keyword, lhs, value.get_operand());
    else
        return fmt::format_to(context.out(), "({} {} {} {})", dl::RoleProjectSyntaxTag::keyword, lhs, *it, value.get_operand());
}

template<dl::CategoryTag Category>
fmt::format_context::iterator fmt::formatter<urd::index_view<dl::Argument<Category>, dl::ConstructorRepository>>::format(
    const urd::index_view<dl::Argument<Category>, dl::ConstructorRepository>& value,
    fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value.get_name()); }

template<dl::ConceptOrRoleTag Category>
fmt::format_context::iterator fmt::formatter<urd::index_view<dl::Register<Category>, dl::ConstructorRepository>>::format(
    const urd::index_view<dl::Register<Category>, dl::ConstructorRepository>& value,
    fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value.get_name()); }

template<dl::CategoryTag Category>
fmt::format_context::iterator fmt::formatter<urd::index_view<dl::Constructor<Category>, dl::ConstructorRepository>>::format(
    const urd::index_view<dl::Constructor<Category>, dl::ConstructorRepository>& value,
    fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value.get_variant()); }

template<dl::CategoryTag Category>
fmt::format_context::iterator fmt::formatter<urd::index_view<ds::Denotation<Category>, ds::DenotationRepository>>::format(
    const urd::index_view<ds::Denotation<Category>, ds::DenotationRepository>& value,
    fmt::format_context& context) const
{
    if constexpr (std::same_as<Category, dl::BooleanTag>)
        return fmt::format_to(context.out(), "{}", value.get_value() ? dl::TrueTag::keyword : dl::FalseTag::keyword);
    else if constexpr (std::same_as<Category, dl::NumericalTag>)
        return fmt::format_to(context.out(), "{}", value.get_value());
    else if constexpr (std::same_as<Category, dl::ConceptTag>)
        return fmt::format_to(context.out(), "{}{}", Category::keyword, value.value());
    else if constexpr (std::same_as<Category, dl::RoleTag>)
    {
        auto out = fmt::format_to(context.out(), "{}{{", Category::keyword);
        auto separator = std::string_view {};
        for (auto [source, target] : value.views())
        {
            out = fmt::format_to(out, "{}({}, {})", separator, source.index(), target.index());
            separator = ", ";
        }
        return fmt::format_to(out, "}}");
    }
    else
        static_assert(false, "unhandled DL denotation category");
}

// Query<> and each query constructor tag (dl::QueryConstructorTags).
template struct fmt::formatter<urd::index_view<dl::Query<>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::AtomicStateTag<tyr::formalism::StaticTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::AtomicStateTag<tyr::formalism::FluentTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QueryConceptTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QueryRoleTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QueryJoinTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QueryProjectTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QueryRenameTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QuerySelectEqualTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QuerySelectValueTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QueryUnionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QueryDifferenceTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QueryGenericJoinTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QueryEmptyTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QueryBooleanTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QueryNumericalTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Query<dl::QueryHeadTag>, dl::ConstructorRepository>>;

// Concept and role (dl::ConceptOrRoleTags).
template struct fmt::formatter<urd::index_view<dl::QueryProjection<dl::ConceptTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::QueryProjection<dl::RoleTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Register<dl::ConceptTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Register<dl::RoleTag>, dl::ConstructorRepository>>;

// Each category (dl::CategoryTags).
template struct fmt::formatter<urd::index_view<dl::Argument<dl::ConceptTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Argument<dl::RoleTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Argument<dl::BooleanTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Argument<dl::NumericalTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Constructor<dl::ConceptTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Constructor<dl::RoleTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Constructor<dl::BooleanTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Constructor<dl::NumericalTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<ds::Denotation<dl::ConceptTag>, ds::DenotationRepository>>;
template struct fmt::formatter<urd::index_view<ds::Denotation<dl::RoleTag>, ds::DenotationRepository>>;
template struct fmt::formatter<urd::index_view<ds::Denotation<dl::BooleanTag>, ds::DenotationRepository>>;
template struct fmt::formatter<urd::index_view<ds::Denotation<dl::NumericalTag>, ds::DenotationRepository>>;
