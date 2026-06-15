#ifndef RUNIR_SEMANTICS_FORMATTER_HPP_
#define RUNIR_SEMANTICS_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/formatter.hpp"
#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/uns/declarations.hpp"
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
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::dl::semantics::format
{

inline std::string boolean(bool value) { return value ? runir::kr::dl::TrueTag::keyword : runir::kr::dl::FalseTag::keyword; }

template<typename... Components>
std::string constructor(std::string_view keyword, Components&&... components)
{
    auto os = std::ostringstream {};
    os << fmt::format("@{}", keyword);
    {
        ygg::IndentScope scope(os);
        ((os << '\n' << ygg::print_indent << fmt::format("{}", std::forward<Components>(components))), ...);
    }
    return os.str();
}

template<typename Objects>
std::string constructor_with_objects(std::string_view keyword, Objects objects)
{
    auto os = std::ostringstream {};
    os << fmt::format("@{}", keyword);
    {
        ygg::IndentScope scope(os);
        for (auto object : objects)
            os << '\n' << ygg::print_indent << fmt::format("{:?}", std::string(object.get_name().str()));
    }
    return os.str();
}

template<typename Head, typename Objects>
std::string constructor_with_objects(std::string_view keyword, Head&& head, Objects objects)
{
    auto os = std::ostringstream {};
    os << fmt::format("@{}", keyword);
    {
        ygg::IndentScope scope(os);
        os << '\n' << ygg::print_indent << fmt::format("{}", std::forward<Head>(head));
        for (auto object : objects)
            os << '\n' << ygg::print_indent << fmt::format("{:?}", std::string(object.get_name().str()));
    }
    return os.str();
}

template<runir::kr::dl::CategoryTag Category>
std::string argument_identifier(runir::kr::dl::ArgumentIdentifier<Category> identifier)
{
    return fmt::format("@argument {}", ygg::uint_t(identifier));
}

template<runir::kr::dl::CategoryTag Category>
std::string register_identifier(runir::kr::dl::RegisterIdentifier<Category> identifier)
{
    return fmt::format("@register {}", ygg::uint_t(identifier));
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
        return constructor(runir::kr::dl::grammar::ast::ConceptBot<Family>::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptTop<Family>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptAtomicState<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptAtomicGoal<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptIntersection<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptUnion<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptNegation<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptValueRestriction<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptExistentialQuantification<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<Family>::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<Family>::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<Family>::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<Family>::keyword,
                           view.get_n(),
                           view.get_role(),
                           view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<Family>::keyword,
                           view.get_n(),
                           view.get_role(),
                           view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<Family>::keyword,
                           view.get_n(),
                           view.get_role(),
                           view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptRoleValueMap<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AgreementTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptAgreement<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
        return constructor_with_objects(runir::kr::dl::grammar::ast::ConceptRoleFillers<Family>::keyword, view.get_role(), view.get_objects());
    else if constexpr (std::same_as<Tag, runir::kr::dl::OneOfTag>)
        return constructor_with_objects(runir::kr::dl::grammar::ast::ConceptOneOf<Family>::keyword, view.get_objects());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return constructor(runir::kr::dl::grammar::ast::ConceptNominal<Family>::keyword, fmt::format("{:?}", std::string(view.get_object().get_name().str())));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RegisterTag>)
        return register_identifier(view.get_data().identifier);
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>)
        return argument_identifier(view.get_data().identifier);
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
        return constructor(runir::kr::dl::grammar::ast::RoleUniversal<Family>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor(runir::kr::dl::grammar::ast::RoleAtomicState<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor(runir::kr::dl::grammar::ast::RoleAtomicGoal<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return constructor(runir::kr::dl::grammar::ast::RoleIntersection<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return constructor(runir::kr::dl::grammar::ast::RoleUnion<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return constructor(runir::kr::dl::grammar::ast::RoleComplement<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return constructor(runir::kr::dl::grammar::ast::RoleInverse<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return constructor(runir::kr::dl::grammar::ast::RoleComposition<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return constructor(runir::kr::dl::grammar::ast::RoleTransitiveClosure<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return constructor(runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return constructor(runir::kr::dl::grammar::ast::RoleRestriction<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return constructor(runir::kr::dl::grammar::ast::RoleIdentity<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RegisterTag>)
        return register_identifier(view.get_data().identifier);
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>)
        return argument_identifier(view.get_data().identifier);
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
        return constructor(runir::kr::dl::grammar::ast::BooleanAtomicState<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor(runir::kr::dl::grammar::ast::BooleanAtomicGoal<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return constructor(runir::kr::dl::grammar::ast::BooleanNonempty<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>)
        return argument_identifier(view.get_data().identifier);
    else if constexpr (runir::kr::dl::ComparisonTag<Tag>)
        return constructor(runir::kr::dl::comparison_keyword<Tag>(), view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::BooleanConstantTag>)
        return constructor(runir::kr::dl::boolean_constant_keyword, boolean(view.get_value()));
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
        return constructor(runir::kr::dl::grammar::ast::NumericalCount<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return constructor(runir::kr::dl::grammar::ast::NumericalDistance<Family>::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>)
        return argument_identifier(view.get_data().identifier);
    else if constexpr (std::same_as<Tag, runir::kr::dl::NumericalConstantTag>)
        return constructor(runir::kr::dl::numerical_constant_keyword, view.get_value());
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

#if RUNIR_ENABLE_FMT_FORMATTERS
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

#endif
