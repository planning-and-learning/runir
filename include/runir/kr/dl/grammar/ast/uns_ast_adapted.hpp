#ifndef RUNIR_KR_DL_GRAMMAR_AST_UNS_AST_ADAPTED_HPP_
#define RUNIR_KR_DL_GRAMMAR_AST_UNS_AST_ADAPTED_HPP_

#include "runir/kr/dl/grammar/ast/uns_ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

#define RUNIR_UNS_ADAPT_BOOLEAN_COMPARISON(NodeName)                                                                                          \
    BOOST_FUSION_ADAPT_TPL_STRUCT((Family),                                                                                                   \
                                  (runir::kr::dl::grammar::ast::NodeName)(Family),                                                            \
                                  (runir::kr::dl::grammar::ast::BooleanChoice<Family>, lhs)(runir::kr::dl::grammar::ast::BooleanChoice<Family>, rhs))

#define RUNIR_UNS_ADAPT_NUMERICAL_COMPARISON(NodeName)                                                                                        \
    BOOST_FUSION_ADAPT_TPL_STRUCT((Family),                                                                                                   \
                                  (runir::kr::dl::grammar::ast::NodeName)(Family),                                                            \
                                  (runir::kr::dl::grammar::ast::NumericalChoice<Family>, lhs)(runir::kr::dl::grammar::ast::NumericalChoice<Family>, rhs))

RUNIR_UNS_ADAPT_BOOLEAN_COMPARISON(BooleanEq)
RUNIR_UNS_ADAPT_BOOLEAN_COMPARISON(BooleanNeq)
RUNIR_UNS_ADAPT_BOOLEAN_COMPARISON(BooleanLt)
RUNIR_UNS_ADAPT_BOOLEAN_COMPARISON(BooleanLe)
RUNIR_UNS_ADAPT_BOOLEAN_COMPARISON(BooleanGt)
RUNIR_UNS_ADAPT_BOOLEAN_COMPARISON(BooleanGe)

RUNIR_UNS_ADAPT_NUMERICAL_COMPARISON(NumericalEq)
RUNIR_UNS_ADAPT_NUMERICAL_COMPARISON(NumericalNeq)
RUNIR_UNS_ADAPT_NUMERICAL_COMPARISON(NumericalLt)
RUNIR_UNS_ADAPT_NUMERICAL_COMPARISON(NumericalLe)
RUNIR_UNS_ADAPT_NUMERICAL_COMPARISON(NumericalGt)
RUNIR_UNS_ADAPT_NUMERICAL_COMPARISON(NumericalGe)

#undef RUNIR_UNS_ADAPT_BOOLEAN_COMPARISON
#undef RUNIR_UNS_ADAPT_NUMERICAL_COMPARISON

BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::BooleanConstant)(Family), (bool, value))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::NumericalConstant)(Family), (ygg::uint_t, value))

#endif
