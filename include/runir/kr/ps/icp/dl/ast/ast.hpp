#ifndef RUNIR_KR_PS_ICP_DL_AST_AST_HPP_
#define RUNIR_KR_PS_ICP_DL_AST_AST_HPP_

#include "runir/kr/ps/ext/dl/ast/ast.hpp"

namespace runir::kr::ps::icp::dl::ast
{

namespace x3 = boost::spirit::x3;
namespace ext_ast = runir::kr::ps::ext::dl::ast;
using ext_ast::Condition;
using ext_ast::Effect;
using ext_ast::Feature;
using ext_ast::FeatureVariant;
using ext_ast::Identifier;
using ext_ast::LoadRule;
using ext_ast::NamedValue;
using ext_ast::QueryFeature;
using ext_ast::Register;
using ext_ast::RegisterVariant;

struct ArgumentObject : x3::position_tagged
{
    Identifier name;
};

struct RegisterObject : x3::position_tagged
{
    Identifier name;
};

using Object = ext_ast::PositionedVariant<ArgumentObject, RegisterObject>;

struct XCondition : x3::position_tagged
{
    bool belongs;
    Object object;
    Identifier feature;
};

struct XEffect : x3::position_tagged
{
    bool enter;
    Object object;
    Identifier feature;
};

struct Crule : x3::position_tagged
{
    Identifier action;
    std::vector<Identifier> arguments;
    std::vector<Condition> conditions;
    std::vector<XCondition> xconditions;
    std::vector<XEffect> xeffects;
    std::vector<Effect> effects;
};

using Rule = ext_ast::PositionedVariant<LoadRule<runir::kr::dl::ConceptTag>, LoadRule<runir::kr::dl::RoleTag>, Crule>;

struct RuleEntry : x3::position_tagged
{
    Identifier symbol;
    Identifier source;
    Identifier target;
    std::vector<Rule> rules;
};

struct ResetPair : x3::position_tagged
{
    Identifier before;
    Identifier after;
};

struct Module : x3::position_tagged
{
    Identifier name;
    std::vector<RegisterVariant> registers;
    Identifier entry;
    std::vector<NamedValue> memory_states;
    std::vector<FeatureVariant> features;
    std::vector<RuleEntry> rule_entries;
    std::vector<ResetPair> reset_pairs;
};

struct Program : x3::position_tagged
{
    Identifier entry;
    Module module;
};

}  // namespace runir::kr::ps::icp::dl::ast

#endif
