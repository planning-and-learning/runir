#ifndef RUNIR_KR_DL_GRAMMAR_AST_UNS_AST_HPP_
#define RUNIR_KR_DL_GRAMMAR_AST_UNS_AST_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/uns/declarations.hpp"

namespace runir::kr::dl::grammar::ast
{

template<runir::kr::dl::FamilyTag Family>
using BooleanChoice = ConstructorOrNonTerminal<Family, runir::kr::dl::BooleanTag>;

template<runir::kr::dl::FamilyTag Family>
using NumericalChoice = ConstructorOrNonTerminal<Family, runir::kr::dl::NumericalTag>;

/**
 * Comparison nodes: one node per (operator, operand category). Each holds two operands of its
 * operand category (as choices, allowing non-terminals). The keyword is the canonical DL keyword.
 *
 * Concrete (rather than Operand-templated) nodes are used so the member type is comma-free, which
 * Boost.Fusion's ADAPT_TPL_STRUCT macro requires.
 */

#define RUNIR_UNS_BOOLEAN_COMPARISON_NODE(NodeName, OpTag)                                              \
    template<runir::kr::dl::FamilyTag Family>                                                          \
    struct NodeName : x3::position_tagged                                                              \
    {                                                                                                  \
        static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::OpTag<runir::kr::dl::BooleanTag>>();   \
        BooleanChoice<Family> lhs;                                                                     \
        BooleanChoice<Family> rhs;                                                                     \
    }

#define RUNIR_UNS_NUMERICAL_COMPARISON_NODE(NodeName, OpTag)                                            \
    template<runir::kr::dl::FamilyTag Family>                                                          \
    struct NodeName : x3::position_tagged                                                              \
    {                                                                                                  \
        static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::OpTag<runir::kr::dl::NumericalTag>>(); \
        NumericalChoice<Family> lhs;                                                                   \
        NumericalChoice<Family> rhs;                                                                   \
    }

RUNIR_UNS_BOOLEAN_COMPARISON_NODE(BooleanEq, EqTag);
RUNIR_UNS_BOOLEAN_COMPARISON_NODE(BooleanNeq, NeqTag);
RUNIR_UNS_BOOLEAN_COMPARISON_NODE(BooleanLt, LtTag);
RUNIR_UNS_BOOLEAN_COMPARISON_NODE(BooleanLe, LeTag);
RUNIR_UNS_BOOLEAN_COMPARISON_NODE(BooleanGt, GtTag);
RUNIR_UNS_BOOLEAN_COMPARISON_NODE(BooleanGe, GeTag);

RUNIR_UNS_NUMERICAL_COMPARISON_NODE(NumericalEq, EqTag);
RUNIR_UNS_NUMERICAL_COMPARISON_NODE(NumericalNeq, NeqTag);
RUNIR_UNS_NUMERICAL_COMPARISON_NODE(NumericalLt, LtTag);
RUNIR_UNS_NUMERICAL_COMPARISON_NODE(NumericalLe, LeTag);
RUNIR_UNS_NUMERICAL_COMPARISON_NODE(NumericalGt, GtTag);
RUNIR_UNS_NUMERICAL_COMPARISON_NODE(NumericalGe, GeTag);

#undef RUNIR_UNS_BOOLEAN_COMPARISON_NODE
#undef RUNIR_UNS_NUMERICAL_COMPARISON_NODE

template<runir::kr::dl::FamilyTag Family>
struct BooleanConstant : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::boolean_constant_keyword;
    bool value;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalConstant : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::numerical_constant_keyword;
    ygg::uint_t value;
};

/**
 * Boolean/Numerical constructor variants for the Uns family extend the base alternatives with the
 * comparison and constant nodes.
 */

template<>
struct Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag> :
    PositionedVariant<x3::forward_ast<BooleanAtomicState<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanAtomicGoal<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanNonempty<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanEq<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanNeq<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanLt<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanLe<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanGt<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanGe<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalEq<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalNeq<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalLt<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalLe<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalGt<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalGe<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanConstant<runir::kr::UnsFamilyTag>>>
{
    using Base = PositionedVariant<x3::forward_ast<BooleanAtomicState<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanAtomicGoal<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanNonempty<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanEq<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanNeq<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanLt<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanLe<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanGt<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanGe<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalEq<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalNeq<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalLt<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalLe<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalGt<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalGe<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanConstant<runir::kr::UnsFamilyTag>>>;
    using Base::Base;
    using Base::operator=;
};

template<>
struct Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag> :
    PositionedVariant<x3::forward_ast<NumericalCount<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalDistance<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalConstant<runir::kr::UnsFamilyTag>>>
{
    using Base = PositionedVariant<x3::forward_ast<NumericalCount<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalDistance<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalConstant<runir::kr::UnsFamilyTag>>>;
    using Base::Base;
    using Base::operator=;
};

}  // namespace runir::kr::dl::grammar::ast

#endif
