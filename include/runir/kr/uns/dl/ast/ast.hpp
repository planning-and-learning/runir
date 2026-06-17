#ifndef RUNIR_KR_UNS_DL_AST_AST_HPP_
#define RUNIR_KR_UNS_DL_AST_AST_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <string>
#include <vector>

namespace runir::kr::uns::dl::ast
{
namespace x3 = boost::spirit::x3;

struct BooleanFeature : x3::position_tagged
{
    std::string symbol;
    std::string description;
    runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag> feature;
};

// A literal of the DNF: a feature symbol, optionally negated.
struct DnfLiteral : x3::position_tagged
{
    std::string symbol;
    bool negated;
};

struct Classifier : x3::position_tagged
{
    std::string symbol;
    std::string description;
    std::vector<BooleanFeature> features;
    std::vector<std::vector<DnfLiteral>> clauses;  // OR of (AND of literals)
};

}  // namespace runir::kr::uns::dl::ast

#endif
