#ifndef RUNIR_SEMANTICS_FORMATTER_HPP_
#define RUNIR_SEMANTICS_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <cstddef>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formatting/cista_formatters.hpp>
#include <yggdrasil/formatting/dynamic_bitset_formatters.hpp>
#include <yggdrasil/formatting/formatter.hpp>

namespace runir::kr::dl::semantics::format
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
        os << ' ' << fmt::format("{:?}", std::string(object.get_name().str()));
    os << ')';
    return os.str();
}

template<typename Head, typename Objects>
std::string constructor_with_objects(std::string_view keyword, Head&& head, Objects objects)
{
    auto os = std::ostringstream {};
    os << '(' << keyword << ' ' << fmt::format("{}", std::forward<Head>(head));
    for (auto object : objects)
        os << ' ' << fmt::format("{:?}", std::string(object.get_name().str()));
    os << ')';
    return os.str();
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

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
std::string concept_constructor(ygg::View<ygg::Index<runir::kr::dl::FamilyConcept<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return constructor(runir::kr::dl::BotTag::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return constructor(runir::kr::dl::TopTag::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor(runir::kr::dl::ConceptAtomicStateSyntaxTag::keyword, fmt::format("{:?}", std::string(view.get_predicate().get_name().str())));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor(runir::kr::dl::ConceptAtomicGoalSyntaxTag::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
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
        return constructor(runir::kr::dl::NominalTag::keyword, fmt::format("{:?}", std::string(view.get_object().get_name().str())));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RegisterTag>)
        return constructor(runir::kr::dl::ConceptRegisterSyntaxTag::keyword, view.get_register().get_name().str());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>)
        return constructor(runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>::keyword, view.get_argument().get_name().str());
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL concept constructor tag in concept_constructor");
    }
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
std::string role(ygg::View<ygg::Index<runir::kr::dl::FamilyRole<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return constructor(runir::kr::dl::UniversalTag::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor(runir::kr::dl::RoleAtomicStateSyntaxTag::keyword, fmt::format("{:?}", std::string(view.get_predicate().get_name().str())));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor(runir::kr::dl::RoleAtomicGoalSyntaxTag::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
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
        return constructor(runir::kr::dl::RoleRegisterSyntaxTag::keyword, view.get_register().get_name().str());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>)
        return constructor(runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>::keyword, view.get_argument().get_name().str());
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL role constructor tag in role");
    }
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
std::string boolean_constructor(ygg::View<ygg::Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor(runir::kr::dl::BooleanAtomicStateSyntaxTag::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor(runir::kr::dl::BooleanAtomicGoalSyntaxTag::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return constructor(runir::kr::dl::NonemptyTag::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>)
        return constructor(runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>::keyword, view.get_argument().get_name().str());
    else if constexpr (runir::kr::dl::ComparisonTag<Tag>)
        return constructor(Tag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::BooleanConstantTag>)
        return constructor(runir::kr::dl::BooleanConstantTag::keyword, boolean(view.get_value()));
    else if constexpr (runir::kr::dl::LogicalBinaryTag<Tag>)
        return constructor(Tag::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NotTag>)
        return constructor(runir::kr::dl::NotTag::keyword, view.get_arg());
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL boolean constructor tag in boolean_constructor");
    }
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
std::string numerical(ygg::View<ygg::Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return constructor(runir::kr::dl::CountTag::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return constructor(runir::kr::dl::DistanceTag::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>)
        return constructor(runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>::keyword, view.get_argument().get_name().str());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NumericalConstantTag>)
        return constructor(runir::kr::dl::NumericalConstantTag::keyword, view.get_value());
    else if constexpr (runir::kr::dl::NumericalBinaryTag<Tag>)
        return constructor(Tag::keyword, view.get_lhs(), view.get_rhs());
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL numerical constructor tag in numerical");
    }
}

}  // namespace runir::kr::dl::semantics::format

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::FamilyConcept<Family, Tag>>, C>;
template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::FamilyRole<Family, Tag>>, C>;
template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C>;
template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C>;
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C>;
template<runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::semantics::Denotation<Category>>, C>;

}  // namespace ygg

template<runir::kr::dl::CategoryTag Category, typename C, typename Char>
struct fmt::range_format_kind<ygg::View<ygg::Index<runir::kr::dl::semantics::Denotation<Category>>, C>, Char, void> : std::false_type
{
};

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::FamilyConcept<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::FamilyConcept<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::concept_constructor(view), ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::FamilyRole<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::FamilyRole<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::role(view), ctx); }
};

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::boolean_constructor(view), ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::numerical(view), ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(fmt::format("{}", view.get_variant()), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::semantics::Denotation<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::semantics::Denotation<Category>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::denotation(view), ctx);
    }
};

#endif
