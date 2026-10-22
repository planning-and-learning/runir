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

template<typename... Alternatives>
struct PositionedVariant : boost::spirit::x3::position_tagged, boost::spirit::x3::variant<Alternatives...>
{
    using Base = boost::spirit::x3::variant<Alternatives...>;
    using Base::Base;
    using Base::operator=;
};

struct Positive : boost::spirit::x3::position_tagged
{
};

struct Negative : boost::spirit::x3::position_tagged
{
};

struct EqualZero : boost::spirit::x3::position_tagged
{
};

struct GreaterZero : boost::spirit::x3::position_tagged
{
};

struct Increases : boost::spirit::x3::position_tagged
{
};

struct Decreases : boost::spirit::x3::position_tagged
{
};

struct Unchanged : boost::spirit::x3::position_tagged
{
};

template<runir::kr::FamilyTag Family>
struct Concept : boost::spirit::x3::position_tagged
{
    runir::kr::parser::ast::Identifier symbol;
    runir::kr::dl::grammar::ast::Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag> feature;
};

template<runir::kr::FamilyTag Family>
struct BooleanFeature : boost::spirit::x3::position_tagged
{
    runir::kr::parser::ast::Identifier symbol;
    runir::kr::dl::grammar::ast::Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag> feature;
};

template<runir::kr::FamilyTag Family>
struct NumericalFeature : boost::spirit::x3::position_tagged
{
    runir::kr::parser::ast::Identifier symbol;
    runir::kr::dl::grammar::ast::Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag> feature;
};

template<runir::kr::FamilyTag Family>
struct Feature : PositionedVariant<BooleanFeature<Family>, NumericalFeature<Family>>
{
    using Base = PositionedVariant<BooleanFeature<Family>, NumericalFeature<Family>>;
    using Base::Base;
    using Base::operator=;
};

struct ConditionObservation : PositionedVariant<Positive, Negative, EqualZero, GreaterZero>
{
    using Base = PositionedVariant<Positive, Negative, EqualZero, GreaterZero>;
    using Base::Base;
    using Base::operator=;
};

struct Condition : boost::spirit::x3::position_tagged
{
    ConditionObservation observation;
    runir::kr::parser::ast::Identifier feature;
};

struct EffectObservation : PositionedVariant<Positive, Negative, Unchanged, Increases, Decreases>
{
    using Base = PositionedVariant<Positive, Negative, Unchanged, Increases, Decreases>;
    using Base::Base;
    using Base::operator=;
};

struct Effect : boost::spirit::x3::position_tagged
{
    EffectObservation observation;
    runir::kr::parser::ast::Identifier feature;
};

template<runir::kr::FamilyTag Family>
struct Rule : boost::spirit::x3::position_tagged
{
    runir::kr::parser::ast::Identifier symbol;
    std::vector<Condition> conditions;
    std::vector<Effect> effects;
};

template<runir::kr::FamilyTag Family>
struct Sketch : boost::spirit::x3::position_tagged
{
    std::vector<Feature<Family>> features;
    std::vector<Rule<Family>> rules;
};

}  // namespace runir::kr::ps::base::dl::ast

#endif
