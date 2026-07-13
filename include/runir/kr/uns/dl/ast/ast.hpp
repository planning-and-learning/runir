#ifndef RUNIR_KR_UNS_DL_AST_AST_HPP_
#define RUNIR_KR_UNS_DL_AST_AST_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/parser/ast.hpp"

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <vector>

namespace runir::kr::uns::dl::ast
{
namespace x3 = boost::spirit::x3;

template<typename... Alternatives>
struct PositionedVariant : x3::position_tagged, x3::variant<Alternatives...>
{
    using Base = x3::variant<Alternatives...>;
    using Base::Base;
    using Base::operator=;
};

struct BooleanFeature : x3::position_tagged
{
    runir::kr::parser::ast::Identifier symbol;
    runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag> feature;
};

struct PositiveLiteral : x3::position_tagged
{
    runir::kr::parser::ast::Identifier symbol;
};

struct NegativeLiteral : x3::position_tagged
{
    runir::kr::parser::ast::Identifier symbol;
};

using Literal = PositionedVariant<PositiveLiteral, NegativeLiteral>;

struct Conjunction : x3::position_tagged
{
    std::vector<Literal> literals;
};

struct Disjunction : x3::position_tagged
{
    std::vector<Conjunction> clauses;
};

struct Classifier : x3::position_tagged
{
    runir::kr::parser::ast::Identifier symbol;
    std::vector<BooleanFeature> features;
    Disjunction expression;
};

}  // namespace runir::kr::uns::dl::ast

#endif
