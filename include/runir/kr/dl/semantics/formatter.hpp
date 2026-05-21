#ifndef RUNIR_SEMANTICS_FORMATTER_HPP_
#define RUNIR_SEMANTICS_FORMATTER_HPP_

#include "runir/common/config.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <cstddef>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <string_view>
#include <tyr/common/formatter.hpp>
#include <vector>

namespace runir::kr::dl::semantics::format
{

template<typename String>
std::string quoted(const String& value)
{
    return fmt::format("\"{}\"", value.str());
}

inline std::string boolean(bool value) { return value ? runir::kr::dl::TrueTag::keyword : runir::kr::dl::FalseTag::keyword; }

template<typename Objects>
std::vector<std::string> quoted_object_names(Objects objects)
{
    auto result = std::vector<std::string> {};
    for (auto object : objects)
        result.push_back(quoted(object.get_name()));
    return result;
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string denotation(tyr::View<tyr::Index<runir::kr::dl::semantics::Denotation<Category>>, C> view)
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
}

template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
std::string concept_constructor(tyr::View<tyr::Index<runir::kr::dl::Concept<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return fmt::format("@{}", runir::kr::dl::grammar::ast::ConceptBot::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return fmt::format("@{}", runir::kr::dl::grammar::ast::ConceptTop::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::ConceptAtomicState::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("@{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAtomicGoal::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptIntersection::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptUnion::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::ConceptNegation::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptValueRestriction::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptExistentialQuantification::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptExactNumberRestriction::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>)
        return fmt::format("@{} {} {} {}", runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction::keyword, view.get_n(), view.get_role(), view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>)
        return fmt::format("@{} {} {} {}", runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction::keyword, view.get_n(), view.get_role(), view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
        return fmt::format("@{} {} {} {}", runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction::keyword, view.get_n(), view.get_role(), view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptRoleValueMap::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AgreementTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptAgreement::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptRoleFillers::keyword, view.get_role(), fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::OneOfTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::ConceptOneOf::keyword, fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::ConceptNominal::keyword, quoted(view.get_object().get_name()));
}

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
std::string role(tyr::View<tyr::Index<runir::kr::dl::Role<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return fmt::format("@{}", runir::kr::dl::grammar::ast::RoleUniversal::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleAtomicState::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("@{} {} {}",
                           runir::kr::dl::grammar::ast::RoleAtomicGoal::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::RoleIntersection::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::RoleUnion::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleComplement::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleInverse::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::RoleComposition::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleTransitiveClosure::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::RoleRestriction::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleIdentity::keyword, view.get_arg());
}

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::string boolean_constructor(tyr::View<tyr::Index<runir::kr::dl::Boolean<Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("@{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicState::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("@{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicGoal::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::BooleanNonempty::keyword, view.get_arg());
}

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::string numerical(tyr::View<tyr::Index<runir::kr::dl::Numerical<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::NumericalCount::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return fmt::format("@{} {} {} {}", runir::kr::dl::grammar::ast::NumericalDistance::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::SumPairDistanceTag>)
        return fmt::format("@n_sum_pair_distance {} {} {} {}",
                           view.get_objects(),
                           view.get_start_role(),
                           view.get_traverse_role(),
                           view.get_target_role());
}

}  // namespace runir::kr::dl::semantics::format

namespace tyr
{

template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::Concept<Tag>>, C>;
template<runir::kr::dl::RoleConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::Role<Tag>>, C>;
template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::Boolean<Tag>>, C>;
template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::Numerical<Tag>>, C>;
template<runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::Constructor<Category>>, C>;
template<runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::semantics::Denotation<Category>>, C>;

}  // namespace tyr

#if RUNIR_ENABLE_FMT_FORMATTERS
template<runir::kr::dl::CategoryTag Category, typename C, typename Char>
struct fmt::range_format_kind<tyr::View<tyr::Index<runir::kr::dl::semantics::Denotation<Category>>, C>, Char, void> : std::false_type
{
};

template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::Concept<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::Concept<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::concept_constructor(view), ctx);
    }
};

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::Role<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::Role<Tag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::role(view), ctx); }
};

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::Boolean<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::Boolean<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::boolean_constructor(view), ctx);
    }
};

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::Numerical<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::Numerical<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::numerical(view), ctx);
    }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::Constructor<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::Constructor<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(fmt::format("{}", view.get_variant()), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::semantics::Denotation<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::semantics::Denotation<Category>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::denotation(view), ctx);
    }
};
#endif

#endif
