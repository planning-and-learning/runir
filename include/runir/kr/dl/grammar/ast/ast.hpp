#ifndef RUNIR_GRAMMAR_AST_AST_HPP_
#define RUNIR_GRAMMAR_AST_AST_HPP_

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

#include <boost/optional.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <string>
#include <yggdrasil/core/types.hpp>
#include <vector>

namespace runir::kr::dl::grammar::ast
{
namespace x3 = boost::spirit::x3;

template<typename... Alternatives>
struct PositionedVariant : x3::position_tagged, x3::variant<Alternatives...>
{
    using Base = x3::variant<Alternatives...>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
struct ConceptBot;
template<runir::kr::dl::FamilyTag Family>
struct ConceptTop;
template<runir::kr::dl::FamilyTag Family>
struct ConceptAtomicState;
template<runir::kr::dl::FamilyTag Family>
struct ConceptAtomicGoal;
template<runir::kr::dl::FamilyTag Family>
struct ConceptIntersection;
template<runir::kr::dl::FamilyTag Family>
struct ConceptUnion;
template<runir::kr::dl::FamilyTag Family>
struct ConceptNegation;
template<runir::kr::dl::FamilyTag Family>
struct ConceptValueRestriction;
template<runir::kr::dl::FamilyTag Family>
struct ConceptExistentialQuantification;
template<runir::kr::dl::FamilyTag Family>
struct ConceptAtLeastNumberRestriction;
template<runir::kr::dl::FamilyTag Family>
struct ConceptAtMostNumberRestriction;
template<runir::kr::dl::FamilyTag Family>
struct ConceptExactNumberRestriction;
template<runir::kr::dl::FamilyTag Family>
struct ConceptQualifiedAtLeastNumberRestriction;
template<runir::kr::dl::FamilyTag Family>
struct ConceptQualifiedAtMostNumberRestriction;
template<runir::kr::dl::FamilyTag Family>
struct ConceptQualifiedExactNumberRestriction;
template<runir::kr::dl::FamilyTag Family>
struct ConceptRoleValueMap;
template<runir::kr::dl::FamilyTag Family>
struct ConceptAgreement;
template<runir::kr::dl::FamilyTag Family>
struct ConceptRoleFillers;
template<runir::kr::dl::FamilyTag Family>
struct ConceptOneOf;
template<runir::kr::dl::FamilyTag Family>
struct ConceptNominal;
template<runir::kr::dl::FamilyTag Family>
struct RoleUniversal;
template<runir::kr::dl::FamilyTag Family>
struct RoleAtomicState;
template<runir::kr::dl::FamilyTag Family>
struct RoleAtomicGoal;
template<runir::kr::dl::FamilyTag Family>
struct RoleIntersection;
template<runir::kr::dl::FamilyTag Family>
struct RoleUnion;
template<runir::kr::dl::FamilyTag Family>
struct RoleComplement;
template<runir::kr::dl::FamilyTag Family>
struct RoleInverse;
template<runir::kr::dl::FamilyTag Family>
struct RoleComposition;
template<runir::kr::dl::FamilyTag Family>
struct RoleTransitiveClosure;
template<runir::kr::dl::FamilyTag Family>
struct RoleReflexiveTransitiveClosure;
template<runir::kr::dl::FamilyTag Family>
struct RoleRestriction;
template<runir::kr::dl::FamilyTag Family>
struct RoleIdentity;

template<runir::kr::dl::FamilyTag Family>
struct BooleanAtomicState;
template<runir::kr::dl::FamilyTag Family>
struct BooleanAtomicGoal;
template<runir::kr::dl::FamilyTag Family>
struct BooleanNonempty;

template<runir::kr::dl::FamilyTag Family>
struct NumericalCount;
template<runir::kr::dl::FamilyTag Family>
struct NumericalDistance;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Constructor;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct NonTerminal : x3::position_tagged
{
    std::string name;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct ConstructorOrNonTerminal : PositionedVariant<NonTerminal<Family, Category>, Constructor<Family, Category>>
{
    using Base = PositionedVariant<NonTerminal<Family, Category>, Constructor<Family, Category>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
using ConceptChoice = ConstructorOrNonTerminal<Family, runir::kr::dl::ConceptTag>;

template<runir::kr::dl::FamilyTag Family>
using RoleChoice = ConstructorOrNonTerminal<Family, runir::kr::dl::RoleTag>;

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::ConceptTag> :
    PositionedVariant<x3::forward_ast<ConceptBot<Family>>,
                      x3::forward_ast<ConceptTop<Family>>,
                      x3::forward_ast<ConceptAtomicState<Family>>,
                      x3::forward_ast<ConceptAtomicGoal<Family>>,
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
                      x3::forward_ast<ConceptOneOf<Family>>,
                      x3::forward_ast<ConceptNominal<Family>>>
{
    using Base = PositionedVariant<x3::forward_ast<ConceptBot<Family>>,
                                   x3::forward_ast<ConceptTop<Family>>,
                                   x3::forward_ast<ConceptAtomicState<Family>>,
                                   x3::forward_ast<ConceptAtomicGoal<Family>>,
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
                                   x3::forward_ast<ConceptOneOf<Family>>,
                                   x3::forward_ast<ConceptNominal<Family>>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::RoleTag> :
    PositionedVariant<x3::forward_ast<RoleUniversal<Family>>,
                      x3::forward_ast<RoleAtomicState<Family>>,
                      x3::forward_ast<RoleAtomicGoal<Family>>,
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
    using Base = PositionedVariant<x3::forward_ast<RoleUniversal<Family>>,
                                   x3::forward_ast<RoleAtomicState<Family>>,
                                   x3::forward_ast<RoleAtomicGoal<Family>>,
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
struct Constructor<Family, runir::kr::dl::BooleanTag> :
    PositionedVariant<x3::forward_ast<BooleanAtomicState<Family>>, x3::forward_ast<BooleanAtomicGoal<Family>>, x3::forward_ast<BooleanNonempty<Family>>>
{
    using Base =
        PositionedVariant<x3::forward_ast<BooleanAtomicState<Family>>, x3::forward_ast<BooleanAtomicGoal<Family>>, x3::forward_ast<BooleanNonempty<Family>>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
struct Constructor<Family, runir::kr::dl::NumericalTag> : PositionedVariant<x3::forward_ast<NumericalCount<Family>>, x3::forward_ast<NumericalDistance<Family>>>
{
    using Base = PositionedVariant<x3::forward_ast<NumericalCount<Family>>, x3::forward_ast<NumericalDistance<Family>>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct DerivationRule : x3::position_tagged
{
    using CategoryTag = Category;
    using Lhs = NonTerminal<Family, Category>;
    using Rhs = std::vector<ConstructorOrNonTerminal<Family, Category>>;

    Lhs lhs;
    Rhs rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct ConceptBot : x3::position_tagged
{
    static constexpr auto keyword = "c_bot";
};

template<runir::kr::dl::FamilyTag Family>
struct ConceptTop : x3::position_tagged
{
    static constexpr auto keyword = "c_top";
};

template<runir::kr::dl::FamilyTag Family>
struct ConceptAtomicState : x3::position_tagged
{
    static constexpr auto keyword = "c_atomic_state";
    std::string predicate_name;
};

template<runir::kr::dl::FamilyTag Family>
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
    ygg::uint_t n;
    RoleChoice<Family> role;
};

template<runir::kr::dl::FamilyTag Family>
struct ConceptAtMostNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_at_most";
    ygg::uint_t n;
    RoleChoice<Family> role;
};

template<runir::kr::dl::FamilyTag Family>
struct ConceptExactNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_exactly";
    ygg::uint_t n;
    RoleChoice<Family> role;
};

template<runir::kr::dl::FamilyTag Family>
struct ConceptQualifiedAtLeastNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_at_least";
    ygg::uint_t n;
    RoleChoice<Family> role;
    ConceptChoice<Family> concept_;
};

template<runir::kr::dl::FamilyTag Family>
struct ConceptQualifiedAtMostNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_at_most";
    ygg::uint_t n;
    RoleChoice<Family> role;
    ConceptChoice<Family> concept_;
};

template<runir::kr::dl::FamilyTag Family>
struct ConceptQualifiedExactNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_exactly";
    ygg::uint_t n;
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

template<runir::kr::dl::FamilyTag Family>
struct ConceptOneOf : x3::position_tagged
{
    static constexpr auto keyword = "c_one_of";
    std::vector<std::string> object_names;
};

template<runir::kr::dl::FamilyTag Family>
struct ConceptNominal : x3::position_tagged
{
    static constexpr auto keyword = "c_nominal";
    std::string object_name;
};

template<runir::kr::dl::FamilyTag Family>
struct RoleUniversal : x3::position_tagged
{
    static constexpr auto keyword = "r_universal";
};

template<runir::kr::dl::FamilyTag Family>
struct RoleAtomicState : x3::position_tagged
{
    static constexpr auto keyword = "r_atomic_state";
    std::string predicate_name;
};

template<runir::kr::dl::FamilyTag Family>
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
struct ConstructorOrNonTerminalVariant : PositionedVariant<ConceptChoice<Family>, RoleChoice<Family>>
{
    using Base = PositionedVariant<ConceptChoice<Family>, RoleChoice<Family>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
struct BooleanAtomicState : x3::position_tagged
{
    static constexpr auto keyword = "b_atomic_state";
    std::string predicate_name;
    bool polarity;
};

template<runir::kr::dl::FamilyTag Family>
struct BooleanAtomicGoal : x3::position_tagged
{
    static constexpr auto keyword = "b_atomic_goal";
    std::string predicate_name;
    bool polarity;
};

template<runir::kr::dl::FamilyTag Family>
struct BooleanNonempty : x3::position_tagged
{
    static constexpr auto keyword = "b_nonempty";
    ConstructorOrNonTerminalVariant<Family> arg;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalCount : x3::position_tagged
{
    static constexpr auto keyword = "n_count";
    ConstructorOrNonTerminalVariant<Family> arg;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalDistance : x3::position_tagged
{
    static constexpr auto keyword = "n_distance";
    ConceptChoice<Family> lhs;
    RoleChoice<Family> mid;
    ConceptChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct DerivationRuleVariant :
    PositionedVariant<DerivationRule<Family, runir::kr::dl::ConceptTag>,
                      DerivationRule<Family, runir::kr::dl::RoleTag>,
                      DerivationRule<Family, runir::kr::dl::BooleanTag>,
                      DerivationRule<Family, runir::kr::dl::NumericalTag>>
{
    using Base = PositionedVariant<DerivationRule<Family, runir::kr::dl::ConceptTag>,
                                   DerivationRule<Family, runir::kr::dl::RoleTag>,
                                   DerivationRule<Family, runir::kr::dl::BooleanTag>,
                                   DerivationRule<Family, runir::kr::dl::NumericalTag>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::dl::FamilyTag Family>
struct Grammar : x3::position_tagged
{
    std::vector<DerivationRuleVariant<Family>> rules;
};

template<runir::kr::dl::FamilyTag Family>
using ConceptConstructor = Constructor<Family, runir::kr::dl::ConceptTag>;

template<runir::kr::dl::FamilyTag Family>
using RoleConstructor = Constructor<Family, runir::kr::dl::RoleTag>;

template<runir::kr::dl::FamilyTag Family>
using BooleanConstructor = Constructor<Family, runir::kr::dl::BooleanTag>;

template<runir::kr::dl::FamilyTag Family>
using NumericalConstructor = Constructor<Family, runir::kr::dl::NumericalTag>;

}  // namespace runir::kr::dl::grammar::ast

// The Uns family extends the Boolean/Numerical constructor variants with comparison and constant
// nodes. Pull the specializations in here so every translation unit that sees grammar::ast also
// sees the consistent (extended) Uns variants, avoiding ODR mismatches.
#include "runir/kr/dl/grammar/ast/uns_ast.hpp"

#endif
