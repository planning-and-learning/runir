#ifndef RUNIR_KR_DL_GRAMMAR_AST_UNS_AST_ADAPTED_HPP_
#define RUNIR_KR_DL_GRAMMAR_AST_UNS_AST_ADAPTED_HPP_

#include "runir/kr/dl/grammar/ast/uns_ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

// Boolean comparisons (two boolean choices).
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::BooleanEq)(Family),
                              (runir::kr::dl::grammar::ast::BooleanChoice<Family>, lhs)(runir::kr::dl::grammar::ast::BooleanChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::BooleanNeq)(Family),
                              (runir::kr::dl::grammar::ast::BooleanChoice<Family>, lhs)(runir::kr::dl::grammar::ast::BooleanChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::BooleanLt)(Family),
                              (runir::kr::dl::grammar::ast::BooleanChoice<Family>, lhs)(runir::kr::dl::grammar::ast::BooleanChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::BooleanLe)(Family),
                              (runir::kr::dl::grammar::ast::BooleanChoice<Family>, lhs)(runir::kr::dl::grammar::ast::BooleanChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::BooleanGt)(Family),
                              (runir::kr::dl::grammar::ast::BooleanChoice<Family>, lhs)(runir::kr::dl::grammar::ast::BooleanChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::BooleanGe)(Family),
                              (runir::kr::dl::grammar::ast::BooleanChoice<Family>, lhs)(runir::kr::dl::grammar::ast::BooleanChoice<Family>, rhs))

// Numerical comparisons (two numerical choices) -> Boolean.
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalEq)(Family),
                              (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalNeq)(Family),
                              (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalLt)(Family),
                              (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalLe)(Family),
                              (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalGt)(Family),
                              (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalGe)(Family),
                              (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))

// Constants.
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::BooleanConstant)(Family), (bool, value))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::NumericalConstant)(Family), (ygg::uint_t, value))

// Numerical binary operators (two numerical choices) -> Numerical.
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalAdd)(Family),
                              (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalSub)(Family),
                              (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalMul)(Family),
                              (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalDiv)(Family),
                              (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalMin)(Family),
                              (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalMax)(Family),
                              (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))

// Logical operators.
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::BooleanAnd)(Family),
                              (runir::kr::dl::grammar::ast::BooleanChoice<Family>, lhs)(runir::kr::dl::grammar::ast::BooleanChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::BooleanOr)(Family),
                              (runir::kr::dl::grammar::ast::BooleanChoice<Family>, lhs)(runir::kr::dl::grammar::ast::BooleanChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::BooleanNot)(Family), (runir::kr::dl::grammar::ast::BooleanChoice<Family>, arg))

#endif
