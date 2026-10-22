#include <gtest/gtest.h>
#include <runir/kr/dl/grammar/parser/parser.hpp>

namespace runir::tests
{

TEST(RunirTests, UnsFamilyGrammarParserAcceptsComparisonsAndConstants)
{
    // Valid expressions for the new constructors parse without error.
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_numerical_ast<>("(n_const 3)"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_boolean_ast<>("(b_const true)"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_boolean_ast<>("(b_const false)"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_boolean_ast<>("(n_lt (n_count (c_top)) (n_const 3))"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_boolean_ast<>("(n_eq (n_count (c_top)) (n_const 0))"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_boolean_ast<>("(n_ge (n_const 5) (n_const 2))"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_boolean_ast<>("(b_eq (b_const true) (b_const false))"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_boolean_ast<>("(b_neq (b_nonempty (c_top)) (b_const true))"));

    // Base constructors still parse under the uns grammar.
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_numerical_ast<>("(n_count (c_top))"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_boolean_ast<>("(b_nonempty (c_top))"));

    // Malformed expressions are rejected.
    EXPECT_ANY_THROW(runir::kr::dl::grammar::parser::parse_numerical_ast<>("(n_const)"));
    EXPECT_ANY_THROW(runir::kr::dl::grammar::parser::parse_boolean_ast<>("(n_lt (n_const 1))"));
    EXPECT_ANY_THROW(runir::kr::dl::grammar::parser::parse_boolean_ast<>("(b_bogus true)"));

    // New arithmetic / logical / min-max operators parse.
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_numerical_ast<>("(n_add (n_const 1) (n_const 2))"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_numerical_ast<>("(n_sub (n_const 1) (n_const 2))"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_numerical_ast<>("(n_mul (n_count (c_top)) (n_const 2))"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_numerical_ast<>("(n_div (n_const 6) (n_const 0))"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_numerical_ast<>("(n_min (n_const 1) (n_max (n_const 2) (n_const 3)))"));
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_boolean_ast<>("(b_and (b_const true) (b_or (b_const false) (b_not (b_const true))))"));
    EXPECT_ANY_THROW(runir::kr::dl::grammar::parser::parse_numerical_ast<>("(n_add (n_const 1))"));
    EXPECT_ANY_THROW(runir::kr::dl::grammar::parser::parse_boolean_ast<>("(b_not (b_const true) (b_const false))"));
}

TEST(RunirTests, RelationalSyntaxParses)
{
    const auto expression = R"((n_count (q_select_value X "constant" (q_project (X) (q_atomic_goal "triple" true (X Y Z))))))";
    EXPECT_NO_THROW(runir::kr::dl::grammar::parser::parse_numerical_ast<>(expression));
    EXPECT_ANY_THROW(runir::kr::dl::grammar::parser::parse_numerical_ast<>("(n_count q_0)"));
    EXPECT_ANY_THROW(runir::kr::dl::grammar::parser::parse_numerical_ast<>("(n_count (q_join (q_concept X (c_top))))"));
}

}  // namespace runir::tests
