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

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
std::string concept_constructor(tyr::View<tyr::Index<runir::kr::dl::FamilyConcept<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return fmt::format("@{}", runir::kr::dl::grammar::ast::ConceptBot<Family>::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return fmt::format("@{}", runir::kr::dl::grammar::ast::ConceptTop<Family>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::ConceptAtomicState<Family>::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("@{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAtomicGoal<Family>::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptIntersection<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptUnion<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::ConceptNegation<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptValueRestriction<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptExistentialQuantification<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<Family>::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<Family>::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<Family>::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>)
        return fmt::format("@{} {} {} {}",
                           runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<Family>::keyword,
                           view.get_n(),
                           view.get_role(),
                           view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>)
        return fmt::format("@{} {} {} {}",
                           runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<Family>::keyword,
                           view.get_n(),
                           view.get_role(),
                           view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
        return fmt::format("@{} {} {} {}",
                           runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<Family>::keyword,
                           view.get_n(),
                           view.get_role(),
                           view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptRoleValueMap<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AgreementTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptAgreement<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
        return fmt::format("@{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptRoleFillers<Family>::keyword,
                           view.get_role(),
                           fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::OneOfTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::ConceptOneOf<Family>::keyword, fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::ConceptNominal<Family>::keyword, quoted(view.get_object().get_name()));
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::RoleConstructorTag Tag, typename C>
std::string role(tyr::View<tyr::Index<runir::kr::dl::FamilyRole<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return fmt::format("@{}", runir::kr::dl::grammar::ast::RoleUniversal<Family>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleAtomicState<Family>::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("@{} {} {}",
                           runir::kr::dl::grammar::ast::RoleAtomicGoal<Family>::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::RoleIntersection<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::RoleUnion<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleComplement<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleInverse<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::RoleComposition<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleTransitiveClosure<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::RoleRestriction<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleIdentity<Family>::keyword, view.get_arg());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::string boolean_constructor(tyr::View<tyr::Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("@{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicState<Family>::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("@{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicGoal<Family>::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::BooleanNonempty<Family>::keyword, view.get_arg());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::string numerical(tyr::View<tyr::Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::NumericalCount<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return fmt::format("@{} {} {} {}", runir::kr::dl::grammar::ast::NumericalDistance<Family>::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
}

}  // namespace runir::kr::dl::semantics::format

namespace tyr
{

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::FamilyConcept<Family, Tag>>, C>;
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::RoleConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::FamilyRole<Family, Tag>>, C>;
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::BooleanConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C>;
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C>;
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C>;
template<runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::semantics::Denotation<Category>>, C>;

}  // namespace tyr

#if RUNIR_ENABLE_FMT_FORMATTERS
template<runir::kr::dl::CategoryTag Category, typename C, typename Char>
struct fmt::range_format_kind<tyr::View<tyr::Index<runir::kr::dl::semantics::Denotation<Category>>, C>, Char, void> : std::false_type
{
};

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::FamilyConcept<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::FamilyConcept<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::concept_constructor(view), ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::RoleConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::FamilyRole<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::FamilyRole<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::role(view), ctx); }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::BooleanConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::boolean_constructor(view), ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::numerical(view), ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C>;
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
