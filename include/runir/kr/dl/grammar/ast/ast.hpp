#ifndef RUNIR_GRAMMAR_AST_AST_HPP_
#define RUNIR_GRAMMAR_AST_AST_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <boost/optional.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <string>
#include <vector>

namespace runir::kr::dl::grammar::ast
{
namespace x3 = boost::spirit::x3;

struct ConceptBot;
struct ConceptTop;
struct ConceptAtomicState;
struct ConceptAtomicGoal;
struct ConceptIntersection;
struct ConceptUnion;
struct ConceptNegation;
struct ConceptValueRestriction;
struct ConceptExistentialQuantification;
struct ConceptAtLeastNumberRestriction;
struct ConceptAtMostNumberRestriction;
struct ConceptExactNumberRestriction;
struct ConceptQualifiedAtLeastNumberRestriction;
struct ConceptQualifiedAtMostNumberRestriction;
struct ConceptQualifiedExactNumberRestriction;
struct ConceptRoleValueMap;
struct ConceptAgreement;
struct ConceptRoleFillers;
struct ConceptOneOf;
struct ConceptNominal;

struct RoleUniversal;
struct RoleAtomicState;
struct RoleAtomicGoal;
struct RoleIntersection;
struct RoleUnion;
struct RoleComplement;
struct RoleInverse;
struct RoleComposition;
struct RoleTransitiveClosure;
struct RoleReflexiveTransitiveClosure;
struct RoleRestriction;
struct RoleIdentity;

struct BooleanAtomicState;
struct BooleanAtomicGoal;
struct BooleanNonempty;

struct NumericalCount;
struct NumericalDistance;
struct NumericalSumPairDistance;

struct ConstructorOrNonTerminalVariant;
struct DerivationRuleVariant;

struct Grammar;

template<runir::kr::dl::CategoryTag D>
struct Constructor
{
};

template<>
struct Constructor<runir::kr::dl::ConceptTag> :
    x3::position_tagged,
    x3::variant<x3::forward_ast<ConceptBot>,
                x3::forward_ast<ConceptTop>,
                x3::forward_ast<ConceptAtomicState>,
                x3::forward_ast<ConceptAtomicGoal>,
                x3::forward_ast<ConceptIntersection>,
                x3::forward_ast<ConceptUnion>,
                x3::forward_ast<ConceptNegation>,
                x3::forward_ast<ConceptValueRestriction>,
                x3::forward_ast<ConceptExistentialQuantification>,
                x3::forward_ast<ConceptAtLeastNumberRestriction>,
                x3::forward_ast<ConceptAtMostNumberRestriction>,
                x3::forward_ast<ConceptExactNumberRestriction>,
                x3::forward_ast<ConceptQualifiedAtLeastNumberRestriction>,
                x3::forward_ast<ConceptQualifiedAtMostNumberRestriction>,
                x3::forward_ast<ConceptQualifiedExactNumberRestriction>,
                x3::forward_ast<ConceptRoleValueMap>,
                x3::forward_ast<ConceptAgreement>,
                x3::forward_ast<ConceptRoleFillers>,
                x3::forward_ast<ConceptOneOf>,
                x3::forward_ast<ConceptNominal>>
{
    using base_type::base_type;
    using base_type::operator=;
};

template<>
struct Constructor<runir::kr::dl::RoleTag> :
    x3::position_tagged,
    x3::variant<x3::forward_ast<RoleUniversal>,
                x3::forward_ast<RoleAtomicState>,
                x3::forward_ast<RoleAtomicGoal>,
                x3::forward_ast<RoleIntersection>,
                x3::forward_ast<RoleUnion>,
                x3::forward_ast<RoleComplement>,
                x3::forward_ast<RoleInverse>,
                x3::forward_ast<RoleComposition>,
                x3::forward_ast<RoleTransitiveClosure>,
                x3::forward_ast<RoleReflexiveTransitiveClosure>,
                x3::forward_ast<RoleRestriction>,
                x3::forward_ast<RoleIdentity>>
{
    using base_type::base_type;
    using base_type::operator=;
};

template<>
struct Constructor<runir::kr::dl::BooleanTag> :
    x3::position_tagged,
    x3::variant<x3::forward_ast<BooleanAtomicState>, x3::forward_ast<BooleanAtomicGoal>, x3::forward_ast<BooleanNonempty>>
{
    using base_type::base_type;
    using base_type::operator=;
};

template<>
struct Constructor<runir::kr::dl::NumericalTag> :
    x3::position_tagged,
    x3::variant<x3::forward_ast<NumericalCount>, x3::forward_ast<NumericalDistance>, x3::forward_ast<NumericalSumPairDistance>>
{
    using base_type::base_type;
    using base_type::operator=;
};

template<runir::kr::dl::CategoryTag D>
struct NonTerminal : x3::position_tagged
{
    std::string name;
};

template<runir::kr::dl::CategoryTag D>
struct ConstructorOrNonTerminal : x3::position_tagged, x3::variant<NonTerminal<D>, Constructor<D>>
{
    using typename x3::variant<NonTerminal<D>, Constructor<D>>::base_type;
    using base_type::base_type;
    using base_type::operator=;
};

template<runir::kr::dl::CategoryTag D>
struct DerivationRule : x3::position_tagged
{
    using Category = D;

    NonTerminal<D> lhs;
    std::vector<ConstructorOrNonTerminal<D>> rhs;
};

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

struct ConceptIntersection : x3::position_tagged
{
    static constexpr auto keyword = "c_and";

    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> lhs;
    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> rhs;
};

struct ConceptUnion : x3::position_tagged
{
    static constexpr auto keyword = "c_or";

    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> lhs;
    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> rhs;
};

struct ConceptNegation : x3::position_tagged
{
    static constexpr auto keyword = "c_not";

    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> arg;
};

struct ConceptValueRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_all";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> rhs;
};

struct ConceptExistentialQuantification : x3::position_tagged
{
    static constexpr auto keyword = "c_some";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> rhs;
};

struct ConceptAtLeastNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_at_least";

    tyr::uint_t n;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> role;
};

struct ConceptAtMostNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_at_most";

    tyr::uint_t n;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> role;
};

struct ConceptExactNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_exactly";

    tyr::uint_t n;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> role;
};

struct ConceptQualifiedAtLeastNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_at_least";

    tyr::uint_t n;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> role;
    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> concept_;
};

struct ConceptQualifiedAtMostNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_at_most";

    tyr::uint_t n;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> role;
    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> concept_;
};

struct ConceptQualifiedExactNumberRestriction : x3::position_tagged
{
    static constexpr auto keyword = "c_exactly";

    tyr::uint_t n;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> role;
    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> concept_;
};

struct ConceptRoleValueMap : x3::position_tagged
{
    static constexpr auto keyword = "c_subset";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> rhs;
};

struct ConceptAgreement : x3::position_tagged
{
    static constexpr auto keyword = "c_same_as";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> rhs;
};

struct ConceptRoleFillers : x3::position_tagged
{
    static constexpr auto keyword = "c_fillers";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> role;
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

struct RoleIntersection : x3::position_tagged
{
    static constexpr auto keyword = "r_and";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> rhs;
};

struct RoleUnion : x3::position_tagged
{
    static constexpr auto keyword = "r_or";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> rhs;
};

struct RoleComplement : x3::position_tagged
{
    static constexpr auto keyword = "r_complement";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> arg;
};

struct RoleInverse : x3::position_tagged
{
    static constexpr auto keyword = "r_inverse";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> arg;
};

struct RoleComposition : x3::position_tagged
{
    static constexpr auto keyword = "r_composition";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> rhs;
};

struct RoleTransitiveClosure : x3::position_tagged
{
    static constexpr auto keyword = "r_transitive_closure";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> arg;
};

struct RoleReflexiveTransitiveClosure : x3::position_tagged
{
    static constexpr auto keyword = "r_reflexive_transitive_closure";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> arg;
};

struct RoleRestriction : x3::position_tagged
{
    static constexpr auto keyword = "r_restriction";

    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> rhs;
};

struct RoleIdentity : x3::position_tagged
{
    static constexpr auto keyword = "r_identity";

    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> arg;
};

struct ConstructorOrNonTerminalVariant : x3::variant<ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>, ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct BooleanAtomicState : x3::position_tagged
{
    static constexpr auto keyword = "b_atomic_state";

    std::string predicate_name;
    bool polarity;
};

struct BooleanAtomicGoal : x3::position_tagged
{
    static constexpr auto keyword = "b_atomic_goal";

    std::string predicate_name;
    bool polarity;
};

struct BooleanNonempty : x3::position_tagged
{
    static constexpr auto keyword = "b_nonempty";

    ConstructorOrNonTerminalVariant arg;
};

struct NumericalCount : x3::position_tagged
{
    static constexpr auto keyword = "n_count";

    ConstructorOrNonTerminalVariant arg;
};

struct NumericalDistance : x3::position_tagged
{
    static constexpr auto keyword = "n_distance";

    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> lhs;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> mid;
    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> rhs;
};

// (n_sum_pair_distance C R_start R_traverse R_target)
// See SumPairDistanceTag in runir/kr/dl/declarations.hpp.
struct NumericalSumPairDistance : x3::position_tagged
{
    static constexpr auto keyword = "n_sum_pair_distance";

    ConstructorOrNonTerminal<runir::kr::dl::ConceptTag> objects;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> start_role;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> traverse_role;
    ConstructorOrNonTerminal<runir::kr::dl::RoleTag> target_role;
};

struct DerivationRuleVariant :
    x3::position_tagged,
    x3::variant<DerivationRule<runir::kr::dl::ConceptTag>,
                DerivationRule<runir::kr::dl::RoleTag>,
                DerivationRule<runir::kr::dl::BooleanTag>,
                DerivationRule<runir::kr::dl::NumericalTag>>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct Grammar : x3::position_tagged
{
    std::vector<DerivationRuleVariant> rules;
};

}

#endif
