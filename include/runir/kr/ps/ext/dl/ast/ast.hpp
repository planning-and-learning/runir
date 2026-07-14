#ifndef RUNIR_KR_PS_EXT_DL_AST_AST_HPP_
#define RUNIR_KR_PS_EXT_DL_AST_AST_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/parser/ast.hpp"
#include "runir/kr/ps/base/dl/ast/ast.hpp"

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <string>
#include <vector>

namespace runir::kr::ps::ext::dl::ast
{
namespace x3 = boost::spirit::x3;
using Identifier = runir::kr::parser::ast::Identifier;

template<typename... Alternatives>
struct PositionedVariant : x3::position_tagged, x3::variant<Alternatives...>
{
    using Base = x3::variant<Alternatives...>;
    using Base::Base;
    using Base::operator=;
};

using Condition = runir::kr::ps::base::dl::ast::Condition<runir::kr::ExtFamilyTag>;
using Effect = runir::kr::ps::base::dl::ast::Effect<runir::kr::ExtFamilyTag>;

template<runir::kr::dl::CategoryTag Category>
struct Argument : x3::position_tagged
{
    Identifier symbol;
};

using ArgumentVariant = PositionedVariant<Argument<runir::kr::dl::ConceptTag>,
                                          Argument<runir::kr::dl::RoleTag>,
                                          Argument<runir::kr::dl::BooleanTag>,
                                          Argument<runir::kr::dl::NumericalTag>>;

template<runir::kr::dl::CategoryTag Category>
struct Register : x3::position_tagged
{
    Identifier symbol;
};

using RegisterVariant = PositionedVariant<Register<runir::kr::dl::ConceptTag>, Register<runir::kr::dl::RoleTag>>;

struct SymbolExpression : x3::position_tagged
{
    Identifier symbol;
};

template<runir::kr::dl::CategoryTag Category>
struct Feature : x3::position_tagged
{
    Identifier symbol;
    runir::kr::dl::grammar::ast::Constructor<runir::kr::ExtFamilyTag, Category> expression;
};

using FeatureVariant = PositionedVariant<Feature<runir::kr::dl::ConceptTag>,
                                         Feature<runir::kr::dl::RoleTag>,
                                         Feature<runir::kr::dl::BooleanTag>,
                                         Feature<runir::kr::dl::NumericalTag>>;

struct NamedValue : x3::position_tagged
{
    Identifier value;
};

template<runir::kr::dl::CategoryTag Category>
struct LoadRule : x3::position_tagged
{
    std::vector<Condition> conditions;
    Identifier feature;
    Identifier reg;
};

struct SketchRule : x3::position_tagged
{
    std::vector<Condition> conditions;
    std::vector<Effect> effects;
};

struct DoRule : x3::position_tagged
{
    std::vector<Condition> conditions;
    Identifier action;
    std::vector<SymbolExpression> arguments;
    std::vector<Effect> effects;
};

struct CallRule : x3::position_tagged
{
    std::vector<Condition> conditions;
    Identifier callee;
    std::vector<SymbolExpression> arguments;
};

using Rule = PositionedVariant<LoadRule<runir::kr::dl::ConceptTag>, LoadRule<runir::kr::dl::RoleTag>, SketchRule, DoRule, CallRule>;

struct RuleEntry : x3::position_tagged
{
    Identifier symbol;
    Identifier source;
    Identifier target;
    std::vector<Rule> rules;
};

struct Module : x3::position_tagged
{
    Identifier name;
    std::vector<ArgumentVariant> arguments;
    std::vector<RegisterVariant> registers;
    Identifier entry;
    std::vector<NamedValue> memory_states;
    std::vector<FeatureVariant> features;
    std::vector<RuleEntry> rule_entries;
};

struct ModuleProgram : x3::position_tagged
{
    Identifier entry;
    std::vector<Module> modules;
};

}  // namespace runir::kr::ps::ext::dl::ast

#endif
