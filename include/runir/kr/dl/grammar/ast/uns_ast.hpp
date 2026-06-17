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
 */

template<runir::kr::dl::FamilyTag Family>
struct BooleanEq : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::EqTag<runir::kr::dl::BooleanTag>>();
    BooleanChoice<Family> lhs;
    BooleanChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct BooleanNeq : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::NeqTag<runir::kr::dl::BooleanTag>>();
    BooleanChoice<Family> lhs;
    BooleanChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct BooleanLt : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::LtTag<runir::kr::dl::BooleanTag>>();
    BooleanChoice<Family> lhs;
    BooleanChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct BooleanLe : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::LeTag<runir::kr::dl::BooleanTag>>();
    BooleanChoice<Family> lhs;
    BooleanChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct BooleanGt : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::GtTag<runir::kr::dl::BooleanTag>>();
    BooleanChoice<Family> lhs;
    BooleanChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct BooleanGe : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::GeTag<runir::kr::dl::BooleanTag>>();
    BooleanChoice<Family> lhs;
    BooleanChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalEq : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::EqTag<runir::kr::dl::NumericalTag>>();
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalNeq : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::NeqTag<runir::kr::dl::NumericalTag>>();
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalLt : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::LtTag<runir::kr::dl::NumericalTag>>();
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalLe : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::LeTag<runir::kr::dl::NumericalTag>>();
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalGt : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::GtTag<runir::kr::dl::NumericalTag>>();
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalGe : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::comparison_keyword<runir::kr::dl::GeTag<runir::kr::dl::NumericalTag>>();
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

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
 * Numerical binary operator nodes (two numerical choices) and logical operator nodes (binary: two
 * boolean choices; unary not: one boolean choice).
 */

template<runir::kr::dl::FamilyTag Family>
struct NumericalAdd : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::numerical_binary_keyword<runir::kr::dl::AddTag>();
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalSub : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::numerical_binary_keyword<runir::kr::dl::SubtractTag>();
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalMul : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::numerical_binary_keyword<runir::kr::dl::MultiplyTag>();
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalDiv : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::numerical_binary_keyword<runir::kr::dl::DivideTag>();
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalMin : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::numerical_binary_keyword<runir::kr::dl::MinTag>();
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct NumericalMax : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::numerical_binary_keyword<runir::kr::dl::MaxTag>();
    NumericalChoice<Family> lhs;
    NumericalChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct BooleanAnd : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::logical_binary_keyword<runir::kr::dl::AndTag>();
    BooleanChoice<Family> lhs;
    BooleanChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct BooleanOr : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::logical_binary_keyword<runir::kr::dl::OrTag>();
    BooleanChoice<Family> lhs;
    BooleanChoice<Family> rhs;
};

template<runir::kr::dl::FamilyTag Family>
struct BooleanNot : x3::position_tagged
{
    static constexpr auto keyword = runir::kr::dl::logical_not_keyword;
    BooleanChoice<Family> arg;
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
                      x3::forward_ast<BooleanConstant<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanAnd<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanOr<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<BooleanNot<runir::kr::UnsFamilyTag>>>
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
                                   x3::forward_ast<BooleanConstant<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanAnd<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanOr<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<BooleanNot<runir::kr::UnsFamilyTag>>>;
    using Base::Base;
    using Base::operator=;
};

template<>
struct Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag> :
    PositionedVariant<x3::forward_ast<NumericalCount<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalDistance<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalConstant<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalAdd<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalSub<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalMul<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalDiv<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalMin<runir::kr::UnsFamilyTag>>,
                      x3::forward_ast<NumericalMax<runir::kr::UnsFamilyTag>>>
{
    using Base = PositionedVariant<x3::forward_ast<NumericalCount<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalDistance<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalConstant<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalAdd<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalSub<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalMul<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalDiv<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalMin<runir::kr::UnsFamilyTag>>,
                                   x3::forward_ast<NumericalMax<runir::kr::UnsFamilyTag>>>;
    using Base::Base;
    using Base::operator=;
};

}  // namespace runir::kr::dl::grammar::ast

#endif
