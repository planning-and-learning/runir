#ifndef RUNIR_GRAMMAR_AST_AST_HPP_
#define RUNIR_GRAMMAR_AST_AST_HPP_

#include "runir/declarations.hpp"
#include "runir/grammar/ast/declarations.hpp"

#include <boost/optional.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <string>
#include <vector>

namespace runir::grammar::ast
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
struct ConceptRoleValueMapContainment;
struct ConceptRoleValueMapEquality;
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
struct BooleanNonempty;

struct NumericalCount;
struct NumericalDistance;

struct ConstructorOrNonTerminalVariant;
struct DerivationRuleVariant;

struct GrammarHead;
struct GrammarBody;
struct Grammar;

template<runir::CategoryTag D>
struct Constructor
{
};

template<>
struct Constructor<runir::ConceptTag> :
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
                x3::forward_ast<ConceptRoleValueMapContainment>,
                x3::forward_ast<ConceptRoleValueMapEquality>,
                x3::forward_ast<ConceptNominal>>
{
    using base_type::base_type;
    using base_type::operator=;
};

template<>
struct Constructor<runir::RoleTag> :
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
struct Constructor<runir::BooleanTag> : x3::position_tagged, x3::variant<x3::forward_ast<BooleanAtomicState>, x3::forward_ast<BooleanNonempty>>
{
    using base_type::base_type;
    using base_type::operator=;
};

template<>
struct Constructor<runir::NumericalTag> : x3::position_tagged, x3::variant<x3::forward_ast<NumericalCount>, x3::forward_ast<NumericalDistance>>
{
    using base_type::base_type;
    using base_type::operator=;
};

template<runir::CategoryTag D>
struct NonTerminal : x3::position_tagged
{
    std::string name;
};

template<runir::CategoryTag D>
struct ConstructorOrNonTerminal : x3::position_tagged, x3::variant<NonTerminal<D>, Constructor<D>>
{
    using typename x3::variant<NonTerminal<D>, Constructor<D>>::base_type;
    using base_type::base_type;
    using base_type::operator=;
};

template<runir::CategoryTag D>
struct DerivationRule : x3::position_tagged
{
    using Category = D;

    NonTerminal<D> lhs;
    std::vector<ConstructorOrNonTerminal<D>> rhs;
};

struct ConceptBot : x3::position_tagged
{
    static constexpr auto keyword = "concept_bot";
};

struct ConceptTop : x3::position_tagged
{
    static constexpr auto keyword = "concept_top";
};

struct ConceptAtomicState : x3::position_tagged
{
    static constexpr auto keyword = "concept_atomic_state";

    std::string predicate_name;
};

struct ConceptAtomicGoal : x3::position_tagged
{
    static constexpr auto keyword = "concept_atomic_goal";

    std::string predicate_name;
    bool polarity;
};

struct ConceptIntersection : x3::position_tagged
{
    static constexpr auto keyword = "concept_intersection";

    ConstructorOrNonTerminal<runir::ConceptTag> lhs;
    ConstructorOrNonTerminal<runir::ConceptTag> rhs;
};

struct ConceptUnion : x3::position_tagged
{
    static constexpr auto keyword = "concept_union";

    ConstructorOrNonTerminal<runir::ConceptTag> lhs;
    ConstructorOrNonTerminal<runir::ConceptTag> rhs;
};

struct ConceptNegation : x3::position_tagged
{
    static constexpr auto keyword = "concept_negation";

    ConstructorOrNonTerminal<runir::ConceptTag> arg;
};

struct ConceptValueRestriction : x3::position_tagged
{
    static constexpr auto keyword = "concept_value_restriction";

    ConstructorOrNonTerminal<runir::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::ConceptTag> rhs;
};

struct ConceptExistentialQuantification : x3::position_tagged
{
    static constexpr auto keyword = "concept_existential_quantification";

    ConstructorOrNonTerminal<runir::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::ConceptTag> rhs;
};

struct ConceptRoleValueMapContainment : x3::position_tagged
{
    static constexpr auto keyword = "concept_role_value_map_containment";

    ConstructorOrNonTerminal<runir::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::RoleTag> rhs;
};

struct ConceptRoleValueMapEquality : x3::position_tagged
{
    static constexpr auto keyword = "concept_role_value_map_equality";

    ConstructorOrNonTerminal<runir::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::RoleTag> rhs;
};

struct ConceptNominal : x3::position_tagged
{
    static constexpr auto keyword = "concept_nominal";

    std::string object_name;
};

struct RoleUniversal : x3::position_tagged
{
    static constexpr auto keyword = "role_universal";
};

struct RoleAtomicState : x3::position_tagged
{
    static constexpr auto keyword = "role_atomic_state";

    std::string predicate_name;
};

struct RoleAtomicGoal : x3::position_tagged
{
    static constexpr auto keyword = "role_atomic_goal";

    std::string predicate_name;
    bool polarity;
};

struct RoleIntersection : x3::position_tagged
{
    static constexpr auto keyword = "role_intersection";

    ConstructorOrNonTerminal<runir::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::RoleTag> rhs;
};

struct RoleUnion : x3::position_tagged
{
    static constexpr auto keyword = "role_union";

    ConstructorOrNonTerminal<runir::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::RoleTag> rhs;
};

struct RoleComplement : x3::position_tagged
{
    static constexpr auto keyword = "role_complement";

    ConstructorOrNonTerminal<runir::RoleTag> arg;
};

struct RoleInverse : x3::position_tagged
{
    static constexpr auto keyword = "role_inverse";

    ConstructorOrNonTerminal<runir::RoleTag> arg;
};

struct RoleComposition : x3::position_tagged
{
    static constexpr auto keyword = "role_composition";

    ConstructorOrNonTerminal<runir::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::RoleTag> rhs;
};

struct RoleTransitiveClosure : x3::position_tagged
{
    static constexpr auto keyword = "role_transitive_closure";

    ConstructorOrNonTerminal<runir::RoleTag> arg;
};

struct RoleReflexiveTransitiveClosure : x3::position_tagged
{
    static constexpr auto keyword = "role_reflexive_transitive_closure";

    ConstructorOrNonTerminal<runir::RoleTag> arg;
};

struct RoleRestriction : x3::position_tagged
{
    static constexpr auto keyword = "role_restriction";

    ConstructorOrNonTerminal<runir::RoleTag> lhs;
    ConstructorOrNonTerminal<runir::ConceptTag> rhs;
};

struct RoleIdentity : x3::position_tagged
{
    static constexpr auto keyword = "role_identity";

    ConstructorOrNonTerminal<runir::ConceptTag> arg;
};

struct ConstructorOrNonTerminalVariant : x3::variant<ConstructorOrNonTerminal<runir::ConceptTag>, ConstructorOrNonTerminal<runir::RoleTag>>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct BooleanAtomicState : x3::position_tagged
{
    static constexpr auto keyword = "boolean_atomic_state";

    std::string predicate_name;
    bool polarity;
};

struct BooleanNonempty : x3::position_tagged
{
    static constexpr auto keyword = "boolean_nonempty";

    ConstructorOrNonTerminalVariant arg;
};

struct NumericalCount : x3::position_tagged
{
    static constexpr auto keyword = "numerical_count";

    ConstructorOrNonTerminalVariant arg;
};

struct NumericalDistance : x3::position_tagged
{
    static constexpr auto keyword = "numerical_distance";

    ConstructorOrNonTerminal<runir::ConceptTag> lhs;
    ConstructorOrNonTerminal<runir::RoleTag> mid;
    ConstructorOrNonTerminal<runir::ConceptTag> rhs;
};

struct DerivationRuleVariant :
    x3::position_tagged,
    x3::variant<DerivationRule<runir::ConceptTag>, DerivationRule<runir::RoleTag>, DerivationRule<runir::BooleanTag>, DerivationRule<runir::NumericalTag>>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct GrammarHead : x3::position_tagged
{
    boost::optional<NonTerminal<runir::ConceptTag>> concept_start;
    boost::optional<NonTerminal<runir::RoleTag>> role_start;
    boost::optional<NonTerminal<runir::BooleanTag>> boolean_start;
    boost::optional<NonTerminal<runir::NumericalTag>> numerical_start;
};

struct GrammarBody : x3::position_tagged
{
    std::vector<DerivationRuleVariant> rules;
};

struct Grammar : x3::position_tagged
{
    GrammarHead head;
    GrammarBody body;
};

}

#endif
