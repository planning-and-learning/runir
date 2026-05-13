#ifndef RUNIR_KR_GP_DL_AST_AST_HPP_
#define RUNIR_KR_GP_DL_AST_AST_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/gp/dl/declarations.hpp"

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <string>
#include <vector>

namespace runir::kr::gp::dl::ast
{
namespace x3 = boost::spirit::x3;

struct BooleanFeature : x3::position_tagged
{
    std::string name;
    std::string symbol;
    std::string description;
    runir::kr::dl::grammar::ast::Constructor<runir::kr::dl::BooleanTag> feature;
};

struct NumericalFeature : x3::position_tagged
{
    std::string name;
    std::string symbol;
    std::string description;
    runir::kr::dl::grammar::ast::Constructor<runir::kr::dl::NumericalTag> feature;
};

struct Feature : x3::position_tagged, x3::variant<BooleanFeature, NumericalFeature>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct ConditionObservation : x3::variant<Positive, Negative, EqualZero, GreaterZero>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct Condition : x3::position_tagged
{
    ConditionObservation observation;
    std::string feature;
};

struct EffectObservation : x3::variant<Positive, Negative, Unchanged, Increases, Decreases>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct Effect : x3::position_tagged
{
    EffectObservation observation;
    std::string feature;
};

struct Rule : x3::position_tagged
{
    std::vector<Condition> conditions;
    std::vector<Effect> effects;
};

struct Policy : x3::position_tagged
{
    std::vector<Feature> features;
    std::vector<Rule> rules;
};

}  // namespace runir::kr::gp::dl::ast

#endif
