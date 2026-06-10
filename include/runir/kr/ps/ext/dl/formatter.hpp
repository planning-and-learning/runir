#ifndef RUNIR_KR_PS_EXT_DL_FORMATTER_HPP_
#define RUNIR_KR_PS_EXT_DL_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/formatter.hpp"
#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/parser/ext/ast.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <vector>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext::dl::format
{

inline std::string boolean(bool value) { return value ? runir::kr::dl::TrueTag::keyword : runir::kr::dl::FalseTag::keyword; }

template<typename Objects>
std::vector<std::string> quoted_object_names(Objects objects)
{
    auto result = std::vector<std::string> {};
    for (auto object : objects)
        result.push_back(fmt::format("{:?}", std::string(object.get_name().str())));
    return result;
}

template<typename T>
std::string expression(T value);

template<typename Variant>
std::string variant_expression(const Variant& value)
{
    return ygg::visit([](auto child) { return expression(child); }, value);
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
std::string concept_expression(ygg::View<ygg::Index<runir::kr::dl::FamilyConcept<Family, Tag>>, C> view)
{
    namespace ast = runir::kr::dl::grammar::ast;
    namespace ext_ast = runir::kr::dl::grammar::parser::ext::ast;

    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return fmt::format("({})", ast::ConceptBot<Family>::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return fmt::format("({})", ast::ConceptTop<Family>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("({} {})", ast::ConceptAtomicState<Family>::keyword, fmt::format("{:?}", std::string(view.get_predicate().get_name().str())));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("({} {} {})",
                           ast::ConceptAtomicGoal<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("({} {} {})", ast::ConceptIntersection<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("({} {} {})", ast::ConceptUnion<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return fmt::format("({} {})", ast::ConceptNegation<Family>::keyword, expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return fmt::format("({} {} {})", ast::ConceptValueRestriction<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return fmt::format("({} {} {})", ast::ConceptExistentialQuantification<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag>)
        return fmt::format("({} {} {})", ast::ConceptAtLeastNumberRestriction<Family>::keyword, view.get_n(), expression(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>)
        return fmt::format("({} {} {})", ast::ConceptAtMostNumberRestriction<Family>::keyword, view.get_n(), expression(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
        return fmt::format("({} {} {})", ast::ConceptExactNumberRestriction<Family>::keyword, view.get_n(), expression(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>)
        return fmt::format("({} {} {} {})",
                           ast::ConceptQualifiedAtLeastNumberRestriction<Family>::keyword,
                           view.get_n(),
                           expression(view.get_role()),
                           expression(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>)
        return fmt::format("({} {} {} {})",
                           ast::ConceptQualifiedAtMostNumberRestriction<Family>::keyword,
                           view.get_n(),
                           expression(view.get_role()),
                           expression(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
        return fmt::format("({} {} {} {})",
                           ast::ConceptQualifiedExactNumberRestriction<Family>::keyword,
                           view.get_n(),
                           expression(view.get_role()),
                           expression(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapTag>)
        return fmt::format("({} {} {})", ast::ConceptRoleValueMap<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AgreementTag>)
        return fmt::format("({} {} {})", ast::ConceptAgreement<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
        return fmt::format("({} {} {})",
                           ast::ConceptRoleFillers<Family>::keyword,
                           expression(view.get_role()),
                           fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::OneOfTag>)
        return fmt::format("({} {})", ast::ConceptOneOf<Family>::keyword, fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return fmt::format("({} {})", ast::ConceptNominal<Family>::keyword, fmt::format("{:?}", std::string(view.get_object().get_name().str())));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RegisterTag>)
        return fmt::format("({} {})", ext_ast::ConceptRegister::keyword, ygg::uint_t(view.get_data().identifier));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>)
        return fmt::format("({} {})", ext_ast::Argument<runir::kr::dl::ConceptTag>::keyword, ygg::uint_t(view.get_data().identifier));
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
std::string role(ygg::View<ygg::Index<runir::kr::dl::FamilyRole<Family, Tag>>, C> view)
{
    namespace ast = runir::kr::dl::grammar::ast;
    namespace ext_ast = runir::kr::dl::grammar::parser::ext::ast;

    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return fmt::format("({})", ast::RoleUniversal<Family>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("({} {})", ast::RoleAtomicState<Family>::keyword, fmt::format("{:?}", std::string(view.get_predicate().get_name().str())));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("({} {} {})",
                           ast::RoleAtomicGoal<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("({} {} {})", ast::RoleIntersection<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("({} {} {})", ast::RoleUnion<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return fmt::format("({} {})", ast::RoleComplement<Family>::keyword, expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return fmt::format("({} {})", ast::RoleInverse<Family>::keyword, expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return fmt::format("({} {} {})", ast::RoleComposition<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return fmt::format("({} {})", ast::RoleTransitiveClosure<Family>::keyword, expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return fmt::format("({} {})", ast::RoleReflexiveTransitiveClosure<Family>::keyword, expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return fmt::format("({} {} {})", ast::RoleRestriction<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return fmt::format("({} {})", ast::RoleIdentity<Family>::keyword, expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RegisterTag>)
        return fmt::format("({} {})", ext_ast::RoleRegister::keyword, ygg::uint_t(view.get_data().identifier));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>)
        return fmt::format("({} {})", ext_ast::Argument<runir::kr::dl::RoleTag>::keyword, ygg::uint_t(view.get_data().identifier));
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
std::string boolean_constructor(ygg::View<ygg::Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C> view)
{
    namespace ast = runir::kr::dl::grammar::ast;
    namespace ext_ast = runir::kr::dl::grammar::parser::ext::ast;

    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("({} {} {})",
                           ast::BooleanAtomicState<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("({} {} {})",
                           ast::BooleanAtomicGoal<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return fmt::format("({} {})", ast::BooleanNonempty<Family>::keyword, variant_expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>)
        return fmt::format("({} {})", ext_ast::Argument<runir::kr::dl::BooleanTag>::keyword, ygg::uint_t(view.get_data().identifier));
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
std::string numerical(ygg::View<ygg::Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C> view)
{
    namespace ast = runir::kr::dl::grammar::ast;
    namespace ext_ast = runir::kr::dl::grammar::parser::ext::ast;

    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return fmt::format("({} {})", ast::NumericalCount<Family>::keyword, variant_expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return fmt::format("({} {} {} {})",
                           ast::NumericalDistance<Family>::keyword,
                           expression(view.get_lhs()),
                           expression(view.get_mid()),
                           expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>)
        return fmt::format("({} {})", ext_ast::Argument<runir::kr::dl::NumericalTag>::keyword, ygg::uint_t(view.get_data().identifier));
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
std::string constructor(ygg::View<ygg::Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C> view)
{
    return variant_expression(view.get_variant());
}

template<typename T>
std::string expression(T value)
{
    if constexpr (requires { concept_expression(value); })
        return concept_expression(value);
    else if constexpr (requires { role(value); })
        return role(value);
    else if constexpr (requires { boolean_constructor(value); })
        return boolean_constructor(value);
    else if constexpr (requires { numerical(value); })
        return numerical(value);
    else if constexpr (requires { constructor(value); })
        return constructor(value);
    else
        return variant_expression(value);
}

}  // namespace runir::kr::ps::ext::dl::format

#endif
