#include <gtest/gtest.h>
#include <runir/kr/dl/grammar/parser/parser.hpp>

namespace runir::tests
{

TEST(RunirTests, UnsFamilyGrammarParserAcceptsComparisonsAndConstants)
{
    namespace parser = runir::kr::dl::grammar::parser;

    // Valid expressions for the new constructors parse without error.
    EXPECT_NO_THROW(parser::parse_numerical_ast<runir::kr::UnsFamilyTag>("(n_const 3)"));
    EXPECT_NO_THROW(parser::parse_boolean_ast<runir::kr::UnsFamilyTag>("(b_const true)"));
    EXPECT_NO_THROW(parser::parse_boolean_ast<runir::kr::UnsFamilyTag>("(b_const false)"));
    EXPECT_NO_THROW(parser::parse_boolean_ast<runir::kr::UnsFamilyTag>("(n_lt (n_count (c_top)) (n_const 3))"));
    EXPECT_NO_THROW(parser::parse_boolean_ast<runir::kr::UnsFamilyTag>("(n_eq (n_count (c_top)) (n_const 0))"));
    EXPECT_NO_THROW(parser::parse_boolean_ast<runir::kr::UnsFamilyTag>("(n_ge (n_const 5) (n_const 2))"));
    EXPECT_NO_THROW(parser::parse_boolean_ast<runir::kr::UnsFamilyTag>("(b_eq (b_const true) (b_const false))"));
    EXPECT_NO_THROW(parser::parse_boolean_ast<runir::kr::UnsFamilyTag>("(b_neq (b_nonempty (c_top)) (b_const true))"));

    // Base constructors still parse under the uns grammar.
    EXPECT_NO_THROW(parser::parse_numerical_ast<runir::kr::UnsFamilyTag>("(n_count (c_top))"));
    EXPECT_NO_THROW(parser::parse_boolean_ast<runir::kr::UnsFamilyTag>("(b_nonempty (c_top))"));

    // Malformed expressions are rejected.
    EXPECT_ANY_THROW(parser::parse_numerical_ast<runir::kr::UnsFamilyTag>("(n_const)"));
    EXPECT_ANY_THROW(parser::parse_boolean_ast<runir::kr::UnsFamilyTag>("(n_lt (n_const 1))"));
    EXPECT_ANY_THROW(parser::parse_boolean_ast<runir::kr::UnsFamilyTag>("(b_bogus true)"));

    // New arithmetic / logical / min-max operators parse.
    EXPECT_NO_THROW(parser::parse_numerical_ast<runir::kr::UnsFamilyTag>("(n_add (n_const 1) (n_const 2))"));
    EXPECT_NO_THROW(parser::parse_numerical_ast<runir::kr::UnsFamilyTag>("(n_sub (n_const 1) (n_const 2))"));
    EXPECT_NO_THROW(parser::parse_numerical_ast<runir::kr::UnsFamilyTag>("(n_mul (n_count (c_top)) (n_const 2))"));
    EXPECT_NO_THROW(parser::parse_numerical_ast<runir::kr::UnsFamilyTag>("(n_div (n_const 6) (n_const 0))"));
    EXPECT_NO_THROW(parser::parse_numerical_ast<runir::kr::UnsFamilyTag>("(n_min (n_const 1) (n_max (n_const 2) (n_const 3)))"));
    EXPECT_NO_THROW(parser::parse_boolean_ast<runir::kr::UnsFamilyTag>("(b_and (b_const true) (b_or (b_const false) (b_not (b_const true))))"));
    EXPECT_ANY_THROW(parser::parse_numerical_ast<runir::kr::UnsFamilyTag>("(n_add (n_const 1))"));
    EXPECT_ANY_THROW(parser::parse_boolean_ast<runir::kr::UnsFamilyTag>("(b_not (b_const true) (b_const false))"));
}

}  // namespace runir::tests
