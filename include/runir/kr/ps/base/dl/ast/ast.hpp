#ifndef RUNIR_KR_PS_BASE_DL_AST_AST_HPP_
#define RUNIR_KR_PS_BASE_DL_AST_AST_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/parser/ast.hpp"

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <vector>

namespace runir::kr::ps::base::dl::ast
{
namespace x3 = boost::spirit::x3;

template<typename... Alternatives>
struct PositionedVariant : x3::position_tagged, x3::variant<Alternatives...>
{
    using Base = x3::variant<Alternatives...>;
    using Base::Base;
    using Base::operator=;
};

struct Positive : x3::position_tagged
{
};

struct Negative : x3::position_tagged
{
};

struct EqualZero : x3::position_tagged
{
};

struct GreaterZero : x3::position_tagged
{
};

struct Increases : x3::position_tagged
{
};

struct Decreases : x3::position_tagged
{
};

struct Unchanged : x3::position_tagged
{
};

template<runir::kr::FamilyTag Family>
struct Concept : x3::position_tagged
{
    runir::kr::parser::ast::Identifier symbol;
    runir::kr::dl::grammar::ast::Constructor<Family, runir::kr::dl::ConceptTag> feature;
};

template<runir::kr::FamilyTag Family>
struct BooleanFeature : x3::position_tagged
{
    runir::kr::parser::ast::Identifier symbol;
    runir::kr::dl::grammar::ast::Constructor<Family, runir::kr::dl::BooleanTag> feature;
};

template<runir::kr::FamilyTag Family>
struct NumericalFeature : x3::position_tagged
{
    runir::kr::parser::ast::Identifier symbol;
    runir::kr::dl::grammar::ast::Constructor<Family, runir::kr::dl::NumericalTag> feature;
};

template<runir::kr::FamilyTag Family>
struct Feature : PositionedVariant<BooleanFeature<Family>, NumericalFeature<Family>>
{
    using Base = PositionedVariant<BooleanFeature<Family>, NumericalFeature<Family>>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::FamilyTag Family>
struct ConditionObservation : PositionedVariant<Positive, Negative, EqualZero, GreaterZero>
{
    using Base = PositionedVariant<Positive, Negative, EqualZero, GreaterZero>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::FamilyTag Family>
struct Condition : x3::position_tagged
{
    ConditionObservation<Family> observation;
    runir::kr::parser::ast::Identifier feature;
};

template<runir::kr::FamilyTag Family>
struct EffectObservation : PositionedVariant<Positive, Negative, Unchanged, Increases, Decreases>
{
    using Base = PositionedVariant<Positive, Negative, Unchanged, Increases, Decreases>;
    using Base::Base;
    using Base::operator=;
};

template<runir::kr::FamilyTag Family>
struct Effect : x3::position_tagged
{
    EffectObservation<Family> observation;
    runir::kr::parser::ast::Identifier feature;
};

template<runir::kr::FamilyTag Family>
struct Rule : x3::position_tagged
{
    runir::kr::parser::ast::Identifier symbol;
    std::vector<Condition<Family>> conditions;
    std::vector<Effect<Family>> effects;
};

template<runir::kr::FamilyTag Family>
struct Sketch : x3::position_tagged
{
    std::vector<Feature<Family>> features;
    std::vector<Rule<Family>> rules;
};

}  // namespace runir::kr::ps::base::dl::ast

#endif
