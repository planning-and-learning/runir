#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_BASE_AST_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_BASE_AST_HPP_

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
#include <tyr/common/types.hpp>
#include <vector>

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
    static constexpr auto keyword = "c_bot";
};
struct ConceptTop : x3::position_tagged
{
    static constexpr auto keyword = "c_top";
};
struct ConceptAtomicState : x3::position_tagged
{
    static constexpr auto keyword = "c_atomic_state";
    std::string predicate_name;
};
struct ConceptAtomicGoal : x3::position_tagged
{
    static constexpr auto keyword = "c_atomic_goal";
    std::string predicate_name;
    bool polarity;
};

template<runir::kr::dl::FamilyTag Family>
struct ConceptIntersection : x3::position_tagged
{
    static constexpr auto keyword = "c_and";
    ConceptChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptUnion : x3::position_tagged
{
    static constexpr auto keyword = "c_or";
    ConceptChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptNegation : x3::position_tagged
{
    static constexpr auto keyword = "c_not";
    ConceptChoice<Family> arg;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptValueRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_all";
    RoleChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptExistentialQuantification : x3::position_tagged
{
    static constexpr auto keyword = "c_some";
    RoleChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptAtLeastNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_at_least";
    tyr::uint_t n;
    RoleChoice<Family> role;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptAtMostNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_at_most";
    tyr::uint_t n;
    RoleChoice<Family> role;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptExactNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_exactly";
    tyr::uint_t n;
    RoleChoice<Family> role;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptQualifiedAtLeastNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_at_least";
    tyr::uint_t n;
    RoleChoice<Family> role;
    ConceptChoice<Family> concept_;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptQualifiedAtMostNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_at_most";
    tyr::uint_t n;
    RoleChoice<Family> role;
    ConceptChoice<Family> concept_;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptQualifiedExactNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_exactly";
    tyr::uint_t n;
    RoleChoice<Family> role;
    ConceptChoice<Family> concept_;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptRoleValueMap : x3::position_tagged
{
    static constexpr auto keyword = "c_subset";
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptAgreement : x3::position_tagged
{
    static constexpr auto keyword = "c_same_as";
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct ConceptRoleFillers : x3::position_tagged
{
    static constexpr auto keyword = "c_fillers";
    RoleChoice<Family> role;
    std::vector<std::string> object_names;
};
struct ConceptOneOf : x3::position_tagged
{
    static constexpr auto keyword = "c_one_of";
    std::vector<std::string> object_names;
};
struct ConceptNominal : x3::position_tagged
{
    static constexpr auto keyword = "c_nominal";
    std::string object_name;
};
struct RoleUniversal : x3::position_tagged
{
    static constexpr auto keyword = "r_universal";
};
struct RoleAtomicState : x3::position_tagged
{
    static constexpr auto keyword = "r_atomic_state";
    std::string predicate_name;
};
struct RoleAtomicGoal : x3::position_tagged
{
    static constexpr auto keyword = "r_atomic_goal";
    std::string predicate_name;
    bool polarity;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleIntersection : x3::position_tagged
{
    static constexpr auto keyword = "r_and";
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleUnion : x3::position_tagged
{
    static constexpr auto keyword = "r_or";
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleComplement : x3::position_tagged
{
    static constexpr auto keyword = "r_complement";
    RoleChoice<Family> arg;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleInverse : x3::position_tagged
{
    static constexpr auto keyword = "r_inverse";
    RoleChoice<Family> arg;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleComposition : x3::position_tagged
{
    static constexpr auto keyword = "r_composition";
    RoleChoice<Family> lhs;
    RoleChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleTransitiveClosure : x3::position_tagged
{
    static constexpr auto keyword = "r_transitive_closure";
    RoleChoice<Family> arg;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleReflexiveTransitiveClosure : x3::position_tagged
{
    static constexpr auto keyword = "r_reflexive_transitive_closure";
    RoleChoice<Family> arg;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleRestriction : x3::position_tagged
{
    static constexpr auto keyword = "r_restriction";
    RoleChoice<Family> lhs;
    ConceptChoice<Family> rhs;
};
template<runir::kr::dl::FamilyTag Family>
struct RoleIdentity : x3::position_tagged
{
    static constexpr auto keyword = "r_identity";
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

#endif
