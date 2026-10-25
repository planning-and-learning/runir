#ifndef RUNIR_SEMANTICS_FORMATTER_HPP_
#define RUNIR_SEMANTICS_FORMATTER_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/declarations.hpp"
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
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formatting/cista_formatters.hpp>
#include <yggdrasil/formatting/dynamic_bitset_formatters.hpp>
#include <yggdrasil/formatting/formatter.hpp>

namespace runir::kr::dl::semantics
{

inline std::string boolean(bool value) { return value ? runir::kr::dl::TrueTag::keyword : runir::kr::dl::FalseTag::keyword; }

template<typename... Components>
std::string constructor(std::string_view keyword, Components&&... components)
{
    auto os = std::ostringstream {};
    os << '(' << keyword;
    ((os << ' ' << fmt::format("{}", std::forward<Components>(components))), ...);
    os << ')';
    return os.str();
}

template<typename Objects>
std::string constructor_with_objects(std::string_view keyword, Objects objects)
{
    auto os = std::ostringstream {};
    os << '(' << keyword;
    for (auto object : objects)
        os << ' ' << fmt::format("{:?}", object.get_name().view());
    os << ')';
    return os.str();
}

template<typename Head, typename Objects>
std::string constructor_with_objects(std::string_view keyword, Head&& head, Objects objects)
{
    auto os = std::ostringstream {};
    os << '(' << keyword << ' ' << fmt::format("{}", std::forward<Head>(head));
    for (auto object : objects)
        os << ' ' << fmt::format("{:?}", object.get_name().view());
    os << ')';
    return os.str();
}

template<typename Columns>
std::string query_columns(Columns columns)
{ return fmt::format("({})", fmt::join(columns, " ")); }

template<typename Tag, typename C>
std::string query(ygg::View<ygg::Index<runir::kr::dl::Query<Tag>>, C> view)
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
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL query constructor tag");
}

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
std::string query_projection(ygg::View<ygg::Index<runir::kr::dl::QueryProjection<Category>>, C> view)
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
std::string denotation(ygg::View<ygg::Index<runir::kr::dl::semantics::Denotation<Category>>, C> view)
{
    if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
    {
        return boolean(view.get());
    }
    else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
    {
        return fmt::format("{}", view.get());
    }
    else if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        return fmt::format("{}{}", Category::name, view.get());
    }
    else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        auto pairs = std::vector<std::string> {};
        for (auto [source, target] : view)
            pairs.push_back(fmt::format("({}, {})", source.get_index(), target.get_index()));
        return fmt::format("{}{{{}}}", Category::name, fmt::join(pairs, ", "));
    }
    else
    {
        static_assert(ygg::dependent_false<Category>::value, "unhandled DL denotation category in denotation");
    }
}

template<typename Tag, typename C>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
std::string concept_constructor(ygg::View<ygg::Index<runir::kr::dl::Concept<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return constructor(runir::kr::dl::BotTag::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return constructor(runir::kr::dl::TopTag::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor(runir::kr::dl::ConceptAtomicStateSyntaxTag::keyword, fmt::format("{:?}", view.get_predicate().get_name().view()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor(runir::kr::dl::ConceptAtomicGoalSyntaxTag::keyword,
                           fmt::format("{:?}", view.get_predicate().get_name().view()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return constructor(runir::kr::dl::ConceptIntersectionSyntaxTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return constructor(runir::kr::dl::ConceptUnionSyntaxTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return constructor(runir::kr::dl::NegationTag::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return constructor(runir::kr::dl::ValueRestrictionTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return constructor(runir::kr::dl::ExistentialQuantificationTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag>)
        return constructor(runir::kr::dl::AtLeastNumberRestrictionTag::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>)
        return constructor(runir::kr::dl::AtMostNumberRestrictionTag::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
        return constructor(runir::kr::dl::ExactNumberRestrictionTag::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>)
        return constructor(runir::kr::dl::QualifiedAtLeastNumberRestrictionTag::keyword, view.get_n(), view.get_role(), view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>)
        return constructor(runir::kr::dl::QualifiedAtMostNumberRestrictionTag::keyword, view.get_n(), view.get_role(), view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
        return constructor(runir::kr::dl::QualifiedExactNumberRestrictionTag::keyword, view.get_n(), view.get_role(), view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapTag>)
        return constructor(runir::kr::dl::RoleValueMapTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AgreementTag>)
        return constructor(runir::kr::dl::AgreementTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
        return constructor_with_objects(runir::kr::dl::RoleFillersTag::keyword, view.get_role(), view.get_objects());
    else if constexpr (std::same_as<Tag, runir::kr::dl::OneOfTag>)
        return constructor_with_objects(runir::kr::dl::OneOfTag::keyword, view.get_objects());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return constructor(runir::kr::dl::NominalTag::keyword, fmt::format("{:?}", view.get_object().get_name().view()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RegisterTag>)
        return constructor(runir::kr::dl::ConceptRegisterSyntaxTag::keyword, view.get_register());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>)
        return constructor(runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>::keyword, view.get_argument());
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL concept constructor tag in concept_constructor");
    }
}

template<typename Tag, typename C>
    requires runir::kr::dl::RoleConstructorTag<Tag>
std::string role(ygg::View<ygg::Index<runir::kr::dl::Role<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return constructor(runir::kr::dl::UniversalTag::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor(runir::kr::dl::RoleAtomicStateSyntaxTag::keyword, fmt::format("{:?}", view.get_predicate().get_name().view()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor(runir::kr::dl::RoleAtomicGoalSyntaxTag::keyword,
                           fmt::format("{:?}", view.get_predicate().get_name().view()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return constructor(runir::kr::dl::RoleIntersectionSyntaxTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return constructor(runir::kr::dl::RoleUnionSyntaxTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return constructor(runir::kr::dl::ComplementTag::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return constructor(runir::kr::dl::InverseTag::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return constructor(runir::kr::dl::CompositionTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return constructor(runir::kr::dl::TransitiveClosureTag::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return constructor(runir::kr::dl::ReflexiveTransitiveClosureTag::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return constructor(runir::kr::dl::RestrictionTag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return constructor(runir::kr::dl::IdentityTag::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RegisterTag>)
        return constructor(runir::kr::dl::RoleRegisterSyntaxTag::keyword, view.get_register());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>)
        return constructor(runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>::keyword, view.get_argument());
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL role constructor tag in role");
    }
}

template<typename Tag, typename C>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
std::string boolean_constructor(ygg::View<ygg::Index<runir::kr::dl::Boolean<Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor(runir::kr::dl::BooleanAtomicStateSyntaxTag::keyword,
                           fmt::format("{:?}", view.get_predicate().get_name().view()),
                           boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor(runir::kr::dl::BooleanAtomicGoalSyntaxTag::keyword,
                           fmt::format("{:?}", view.get_predicate().get_name().view()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return constructor(runir::kr::dl::NonemptyTag::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>)
        return constructor(runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>::keyword, view.get_argument());
    else if constexpr (runir::kr::dl::ComparisonTag<Tag> || runir::kr::dl::LogicalBinaryTag<Tag>)
        return constructor(Tag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::BooleanConstantTag>)
        return constructor(runir::kr::dl::BooleanConstantTag::keyword, boolean(view.get_value()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NotTag>)
        return constructor(runir::kr::dl::NotTag::keyword, view.get_arg());
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL boolean constructor tag in boolean_constructor");
    }
}

template<typename Tag, typename C>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
std::string numerical(ygg::View<ygg::Index<runir::kr::dl::Numerical<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return constructor(runir::kr::dl::CountTag::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return constructor(runir::kr::dl::DistanceTag::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>)
        return constructor(runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>::keyword, view.get_argument());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NumericalConstantTag>)
        return constructor(runir::kr::dl::NumericalConstantTag::keyword, view.get_value());
    else if constexpr (runir::kr::dl::NumericalBinaryTag<Tag>)
        return constructor(Tag::keyword, view.get_lhs(), view.get_rhs());
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL numerical constructor tag in numerical");
    }
}

}  // namespace runir::kr::dl::semantics

template<runir::kr::dl::CategoryTag Category, typename C, typename Char>
struct fmt::range_format_kind<ygg::View<ygg::Index<runir::kr::dl::semantics::Denotation<Category>>, C>, Char, void> : std::false_type
{
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::QueryColumn>, C>> : fmt::formatter<std::string_view>
{
    auto format(const ygg::View<ygg::Index<runir::kr::dl::QueryColumn>, C>& value, format_context& ctx) const
    { return fmt::format_to(ctx.out(), "{}", value.get_name()); }
};

template<typename Tag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::Query<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::Query<Tag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::query(view), ctx); }
};

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::QueryProjection<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::QueryProjection<Category>>, C>;
    auto format(View view, format_context& ctx) const
    { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::query_projection(view), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::Argument<Category>>, C>> : fmt::formatter<std::string_view>
{
    auto format(const ygg::View<ygg::Index<runir::kr::dl::Argument<Category>>, C>& value, format_context& ctx) const
    { return fmt::format_to(ctx.out(), "{}", value.get_name()); }
};

template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::Register<Category>>, C>> : fmt::formatter<std::string_view>
{
    auto format(const ygg::View<ygg::Index<runir::kr::dl::Register<Category>>, C>& value, format_context& ctx) const
    { return fmt::format_to(ctx.out(), "{}", value.get_name()); }
};

template<typename Tag, typename C>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::Concept<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::Concept<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::concept_constructor(view), ctx); }
};

template<typename Tag, typename C>
    requires runir::kr::dl::RoleConstructorTag<Tag>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::Role<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::Role<Tag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::role(view), ctx); }
};

template<typename Tag, typename C>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::Boolean<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::Boolean<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::boolean_constructor(view), ctx); }
};

template<typename Tag, typename C>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::Numerical<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::Numerical<Tag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::numerical(view), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::Constructor<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::Constructor<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(fmt::format("{}", view.get_variant()), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::semantics::Denotation<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::semantics::Denotation<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::denotation(view), ctx); }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::semantics::RegisterValues>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& value, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "RegisterValues(concepts=[{}], roles=[{}])",
                              fmt::join(value.template get<runir::kr::dl::ConceptTag>(), ", "),
                              fmt::join(value.template get<runir::kr::dl::RoleTag>(), ", "));
    }
};

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::semantics::CallArguments>, C>>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }
    auto format(const auto& value, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "CallArguments(concepts={}, roles={}, booleans={}, numericals={})",
                              value.template get<runir::kr::dl::ConceptTag>(),
                              value.template get<runir::kr::dl::RoleTag>(),
                              value.template get<runir::kr::dl::BooleanTag>(),
                              value.template get<runir::kr::dl::NumericalTag>());
    }
};

#endif
