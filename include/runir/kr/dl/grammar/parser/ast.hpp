#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_AST_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_AST_HPP_

#ifndef BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif
#ifndef BOOST_MPL_LIMIT_LIST_SIZE
#define BOOST_MPL_LIMIT_LIST_SIZE 30
#endif
#ifndef BOOST_VARIANT_LIMIT_TYPES
#define BOOST_VARIANT_LIMIT_TYPES 30
#endif

#include "runir/kr/dl/declarations.hpp"

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <string>
#include <vector>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::grammar::parser::base::ast
{
namespace x3 = boost::spirit::x3;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Constructor;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct NonTerminal : x3::position_tagged
{
    std::string name;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct ConstructorOrNonTerminal : x3::position_tagged, x3::variant<NonTerminal<Family, Category>, Constructor<Family, Category>>
{
    using typename x3::variant<NonTerminal<Family, Category>, Constructor<Family, Category>>::base_type;
    using base_type::base_type;
    using base_type::operator=;
};

template<runir::kr::dl::FamilyTag Family>
using ConceptChoice = ConstructorOrNonTerminal<Family, runir::kr::dl::ConceptTag>;

template<runir::kr::dl::FamilyTag Family>
using RoleChoice = ConstructorOrNonTerminal<Family, runir::kr::dl::RoleTag>;

struct ConceptBot : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::BotTag::keyword;
};
struct ConceptTop : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::TopTag::keyword;
};
struct ConceptAtomicState : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ConceptAtomicStateSyntaxTag::keyword;
    std::string predicate_name;
};
struct ConceptAtomicGoal : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ConceptAtomicGoalSyntaxTag::keyword;
    std::string predicate_name;
    bool polarity;
};

template<runir::kr::dl::FamilyTag Family>
struct ConceptIntersection : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ConceptIntersectionSyntaxTag::keyword;
    ConceptChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptUnion : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ConceptUnionSyntaxTag::keyword;
    ConceptChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptNegation : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::NegationTag::keyword;
    ConceptChoice<Family> arg;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptValueRestriction : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ValueRestrictionTag::keyword;
    RoleChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptExistentialQuantification : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ExistentialQuantificationTag::keyword;
    RoleChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptAtLeastNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::AtLeastNumberRestrictionTag::keyword;
    ygg::uint_t n;
    RoleChoice<Family> role;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptAtMostNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::AtMostNumberRestrictionTag::keyword;
    ygg::uint_t n;
    RoleChoice<Family> role;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptExactNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ExactNumberRestrictionTag::keyword;
    ygg::uint_t n;
    RoleChoice<Family> role;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptQualifiedAtLeastNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::AtLeastNumberRestrictionTag::keyword;
    ygg::uint_t n;
    RoleChoice<Family> role;
    ConceptChoice<Family> concept_;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptQualifiedAtMostNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::AtMostNumberRestrictionTag::keyword;
    ygg::uint_t n;
    RoleChoice<Family> role;
    ConceptChoice<Family> concept_;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptQualifiedExactNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ExactNumberRestrictionTag::keyword;
    ygg::uint_t n;
    RoleChoice<Family> role;
    ConceptChoice<Family> concept_;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptRoleValueMap : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleValueMapTag::keyword;
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptAgreement : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::AgreementTag::keyword;
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptRoleFillers : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleFillersTag::keyword;
    RoleChoice<Family> role;
    std::vector<std::string> object_names;
};
struct ConceptOneOf : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::OneOfTag::keyword;
    std::vector<std::string> object_names;
};
struct ConceptNominal : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::NominalTag::keyword;
    std::string object_name;
};
struct RoleUniversal : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::UniversalTag::keyword;
};
struct RoleAtomicState : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleAtomicStateSyntaxTag::keyword;
    std::string predicate_name;
};
struct RoleAtomicGoal : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleAtomicGoalSyntaxTag::keyword;
    std::string predicate_name;
    bool polarity;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleIntersection : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleIntersectionSyntaxTag::keyword;
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleUnion : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleUnionSyntaxTag::keyword;
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleComplement : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ComplementTag::keyword;
    RoleChoice<Family> arg;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleInverse : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::InverseTag::keyword;
    RoleChoice<Family> arg;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleComposition : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::CompositionTag::keyword;
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleTransitiveClosure : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::TransitiveClosureTag::keyword;
    RoleChoice<Family> arg;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleReflexiveTransitiveClosure : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ReflexiveTransitiveClosureTag::keyword;
    RoleChoice<Family> arg;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleRestriction : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RestrictionTag::keyword;
    RoleChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleIdentity : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::IdentityTag::keyword;
    ConceptChoice<Family> arg;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag> :
    x3::position_tagged,
    x3::variant<x3::forward_ast<ConceptBot>,
                x3::forward_ast<ConceptTop>,
                x3::forward_ast<ConceptAtomicState>,
                x3::forward_ast<ConceptAtomicGoal>,
                x3::forward_ast<ConceptIntersection<Family>>,
                x3::forward_ast<ConceptUnion<Family>>,
                x3::forward_ast<ConceptNegation<Family>>,
                x3::forward_ast<ConceptValueRestriction<Family>>,
                x3::forward_ast<ConceptExistentialQuantification<Family>>,
                x3::forward_ast<ConceptAtLeastNumberRestriction<Family>>,
                x3::forward_ast<ConceptAtMostNumberRestriction<Family>>,
                x3::forward_ast<ConceptExactNumberRestriction<Family>>,
                x3::forward_ast<ConceptQualifiedAtLeastNumberRestriction<Family>>,
                x3::forward_ast<ConceptQualifiedAtMostNumberRestriction<Family>>,
                x3::forward_ast<ConceptQualifiedExactNumberRestriction<Family>>,
                x3::forward_ast<ConceptRoleValueMap<Family>>,
                x3::forward_ast<ConceptAgreement<Family>>,
                x3::forward_ast<ConceptRoleFillers<Family>>,
                x3::forward_ast<ConceptOneOf>,
                x3::forward_ast<ConceptNominal>>
{
    using Base = x3::variant<x3::forward_ast<ConceptBot>,
                             x3::forward_ast<ConceptTop>,
                             x3::forward_ast<ConceptAtomicState>,
                             x3::forward_ast<ConceptAtomicGoal>,
                             x3::forward_ast<ConceptIntersection<Family>>,
                             x3::forward_ast<ConceptUnion<Family>>,
                             x3::forward_ast<ConceptNegation<Family>>,
                             x3::forward_ast<ConceptValueRestriction<Family>>,
                             x3::forward_ast<ConceptExistentialQuantification<Family>>,
                             x3::forward_ast<ConceptAtLeastNumberRestriction<Family>>,
                             x3::forward_ast<ConceptAtMostNumberRestriction<Family>>,
                             x3::forward_ast<ConceptExactNumberRestriction<Family>>,
                             x3::forward_ast<ConceptQualifiedAtLeastNumberRestriction<Family>>,
                             x3::forward_ast<ConceptQualifiedAtMostNumberRestriction<Family>>,
                             x3::forward_ast<ConceptQualifiedExactNumberRestriction<Family>>,
                             x3::forward_ast<ConceptRoleValueMap<Family>>,
                             x3::forward_ast<ConceptAgreement<Family>>,
                             x3::forward_ast<ConceptRoleFillers<Family>>,
                             x3::forward_ast<ConceptOneOf>,
                             x3::forward_ast<ConceptNominal>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag> :
    x3::position_tagged,
    x3::variant<x3::forward_ast<RoleUniversal>,
                x3::forward_ast<RoleAtomicState>,
                x3::forward_ast<RoleAtomicGoal>,
                x3::forward_ast<RoleIntersection<Family>>,
                x3::forward_ast<RoleUnion<Family>>,
                x3::forward_ast<RoleComplement<Family>>,
                x3::forward_ast<RoleInverse<Family>>,
                x3::forward_ast<RoleComposition<Family>>,
                x3::forward_ast<RoleTransitiveClosure<Family>>,
                x3::forward_ast<RoleReflexiveTransitiveClosure<Family>>,
                x3::forward_ast<RoleRestriction<Family>>,
                x3::forward_ast<RoleIdentity<Family>>>
{
    using Base = x3::variant<x3::forward_ast<RoleUniversal>,
                             x3::forward_ast<RoleAtomicState>,
                             x3::forward_ast<RoleAtomicGoal>,
                             x3::forward_ast<RoleIntersection<Family>>,
                             x3::forward_ast<RoleUnion<Family>>,
                             x3::forward_ast<RoleComplement<Family>>,
                             x3::forward_ast<RoleInverse<Family>>,
                             x3::forward_ast<RoleComposition<Family>>,
                             x3::forward_ast<RoleTransitiveClosure<Family>>,
                             x3::forward_ast<RoleReflexiveTransitiveClosure<Family>>,
                             x3::forward_ast<RoleRestriction<Family>>,
                             x3::forward_ast<RoleIdentity<Family>>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
using ConceptConstructor = Constructor<Family, runir::kr::dl::ConceptTag>;

template<runir::kr::dl::FamilyTag Family>
using RoleConstructor = Constructor<Family, runir::kr::dl::RoleTag>;

}  // namespace runir::kr::dl::grammar::parser::base::ast

namespace runir::kr::dl::grammar::parser::ext::ast
{
namespace x3 = boost::spirit::x3;

template<runir::kr::dl::CategoryTag Category>
struct Argument : x3::position_tagged
{
    ygg::uint_t identifier;
};

struct ConceptRegister : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ConceptRegisterSyntaxTag::keyword;

    ygg::uint_t identifier;
};

struct RoleRegister : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::RoleRegisterSyntaxTag::keyword;

    ygg::uint_t identifier;
};

template<>
struct Argument<runir::kr::dl::ConceptTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>::keyword;

    ygg::uint_t identifier;
};

template<>
struct Argument<runir::kr::dl::RoleTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>::keyword;

    ygg::uint_t identifier;
};

template<>
struct Argument<runir::kr::dl::BooleanTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>::keyword;

    ygg::uint_t identifier;
};

template<>
struct Argument<runir::kr::dl::NumericalTag> : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>::keyword;

    ygg::uint_t identifier;
};

}  // namespace runir::kr::dl::grammar::parser::ext::ast

namespace runir::kr::dl::grammar::parser::base::ast
{

template<>
struct Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag> :
    x3::position_tagged,
    x3::variant<x3::forward_ast<ConceptBot>,
                x3::forward_ast<ConceptTop>,
                x3::forward_ast<ConceptAtomicState>,
                x3::forward_ast<ConceptAtomicGoal>,
                x3::forward_ast<ConceptIntersection<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptUnion<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptNegation<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptValueRestriction<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptExistentialQuantification<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptAtLeastNumberRestriction<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptAtMostNumberRestriction<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptExactNumberRestriction<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptQualifiedAtLeastNumberRestriction<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptQualifiedAtMostNumberRestriction<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptQualifiedExactNumberRestriction<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptRoleValueMap<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptAgreement<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptRoleFillers<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<ConceptOneOf>,
                x3::forward_ast<ConceptNominal>,
                x3::forward_ast<runir::kr::dl::grammar::parser::ext::ast::ConceptRegister>,
                x3::forward_ast<runir::kr::dl::grammar::parser::ext::ast::Argument<runir::kr::dl::ConceptTag>>>
{
    using Base = x3::variant<x3::forward_ast<ConceptBot>,
                             x3::forward_ast<ConceptTop>,
                             x3::forward_ast<ConceptAtomicState>,
                             x3::forward_ast<ConceptAtomicGoal>,
                             x3::forward_ast<ConceptIntersection<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptUnion<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptNegation<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptValueRestriction<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptExistentialQuantification<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptAtLeastNumberRestriction<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptAtMostNumberRestriction<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptExactNumberRestriction<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptQualifiedAtLeastNumberRestriction<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptQualifiedAtMostNumberRestriction<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptQualifiedExactNumberRestriction<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptRoleValueMap<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptAgreement<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptRoleFillers<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<ConceptOneOf>,
                             x3::forward_ast<ConceptNominal>,
                             x3::forward_ast<runir::kr::dl::grammar::parser::ext::ast::ConceptRegister>,
                             x3::forward_ast<runir::kr::dl::grammar::parser::ext::ast::Argument<runir::kr::dl::ConceptTag>>>;
    using Base::Base;
    using Base::operator=;
};

template<>
struct Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag> :
    x3::position_tagged,
    x3::variant<x3::forward_ast<RoleUniversal>,
                x3::forward_ast<RoleAtomicState>,
                x3::forward_ast<RoleAtomicGoal>,
                x3::forward_ast<RoleIntersection<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<RoleUnion<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<RoleComplement<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<RoleInverse<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<RoleComposition<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<RoleTransitiveClosure<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<RoleReflexiveTransitiveClosure<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<RoleRestriction<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<RoleIdentity<runir::kr::ExtFamilyTag>>,
                x3::forward_ast<runir::kr::dl::grammar::parser::ext::ast::RoleRegister>,
                x3::forward_ast<runir::kr::dl::grammar::parser::ext::ast::Argument<runir::kr::dl::RoleTag>>>
{
    using Base = x3::variant<x3::forward_ast<RoleUniversal>,
                             x3::forward_ast<RoleAtomicState>,
                             x3::forward_ast<RoleAtomicGoal>,
                             x3::forward_ast<RoleIntersection<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<RoleUnion<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<RoleComplement<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<RoleInverse<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<RoleComposition<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<RoleTransitiveClosure<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<RoleReflexiveTransitiveClosure<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<RoleRestriction<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<RoleIdentity<runir::kr::ExtFamilyTag>>,
                             x3::forward_ast<runir::kr::dl::grammar::parser::ext::ast::RoleRegister>,
                             x3::forward_ast<runir::kr::dl::grammar::parser::ext::ast::Argument<runir::kr::dl::RoleTag>>>;
    using Base::Base;
    using Base::operator=;
};

}  // namespace runir::kr::dl::grammar::parser::base::ast

#endif
