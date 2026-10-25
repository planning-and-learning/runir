#ifndef RUNIR_KR_PS_ICP_DL_AST_AST_HPP_
#define RUNIR_KR_PS_ICP_DL_AST_AST_HPP_

#include "runir/kr/ps/ext/dl/ast/ast.hpp"

namespace runir::kr::ps::icp::dl::ast
{

using runir::kr::ps::ext::dl::ast::Condition;
using runir::kr::ps::ext::dl::ast::Effect;
using runir::kr::ps::ext::dl::ast::Feature;
using runir::kr::ps::ext::dl::ast::FeatureVariant;
using runir::kr::ps::ext::dl::ast::Identifier;
using runir::kr::ps::ext::dl::ast::LoadRule;
using runir::kr::ps::ext::dl::ast::NamedValue;
using runir::kr::ps::ext::dl::ast::QueryFeature;
using runir::kr::ps::ext::dl::ast::Register;
using runir::kr::ps::ext::dl::ast::RegisterVariant;

struct ArgumentObject : boost::spirit::x3::position_tagged
{
    Identifier name;
};

struct RegisterObject : boost::spirit::x3::position_tagged
{
    Identifier name;
};

using Object = runir::kr::ps::ext::dl::ast::PositionedVariant<ArgumentObject, RegisterObject>;

struct XCondition : boost::spirit::x3::position_tagged
{
    bool belongs = false;
    Object object;
    Identifier feature;
};

struct XEffect : boost::spirit::x3::position_tagged
{
    bool enter = false;
    Object object;
    Identifier feature;
};

struct Crule : boost::spirit::x3::position_tagged
{
    Identifier action;
    std::vector<Identifier> arguments;
    std::vector<Condition> conditions;
    std::vector<XCondition> xconditions;
    std::vector<XEffect> xeffects;
    std::vector<Effect> effects;
};

using Rule = runir::kr::ps::ext::dl::ast::PositionedVariant<LoadRule<runir::kr::dl::ConceptTag>, LoadRule<runir::kr::dl::RoleTag>, Crule>;

struct RuleEntry : boost::spirit::x3::position_tagged
{
    Identifier symbol;
    Identifier source;
    Identifier target;
    std::vector<Rule> rules;
};

struct ResetPair : boost::spirit::x3::position_tagged
{
    Identifier before;
    Identifier after;
};

struct Module : boost::spirit::x3::position_tagged
{
    Identifier name;
    std::vector<RegisterVariant> registers;
    Identifier entry;
    std::vector<NamedValue> memory_states;
    std::vector<FeatureVariant> features;
    std::vector<RuleEntry> rule_entries;
    std::vector<ResetPair> reset_pairs;
};

struct Program : boost::spirit::x3::position_tagged
{
    Identifier entry;
    Module module;
};

}  // namespace runir::kr::ps::icp::dl::ast

#endif
