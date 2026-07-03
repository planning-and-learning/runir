#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <limits>
#include <runir/datasets/task_class.hpp>
#include <runir/kr/dl/grammar/parser/parser.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/uns/evaluation.hpp>
#include <string>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/planning.hpp>
#include <yggdrasil/execution/onetbb.hpp>

namespace runir::tests
{
namespace
{

namespace fp = tyr::formalism::planning;
namespace p = tyr::planning;
namespace dl = runir::kr::dl;
namespace sem = runir::kr::dl::semantics;
using Uns = runir::kr::UnsFamilyTag;

std::filesystem::path benchmark_prefix() { return std::filesystem::path(RUNIR_ROOT_DIR) / "data" / "planning-benchmarks"; }

// Wrap a boolean constructor in a Boolean-category constructor view.
auto wrap_boolean(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repo, auto boolean_view)
{
    auto data = ygg::Data<dl::Constructor<Uns, dl::BooleanTag>>(boolean_view.get_index());
    return repo.get_or_create(data).first;
}

// Wrap a numerical constructor in a Numerical-category constructor view.
auto wrap_numerical(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repo, auto numerical_view)
{
    auto data = ygg::Data<dl::Constructor<Uns, dl::NumericalTag>>(numerical_view.get_index());
    return repo.get_or_create(data).first;
}

auto boolean_constant(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repo, bool value)
{
    auto data = ygg::Data<dl::Boolean<Uns, dl::BooleanConstantTag>>(value);
    return wrap_boolean(repo, repo.get_or_create(data).first);
}

auto numerical_constant(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repo, ygg::uint_t value)
{
    auto data = ygg::Data<dl::Numerical<Uns, dl::NumericalConstantTag>>(value);
    return wrap_numerical(repo, repo.get_or_create(data).first);
}

template<dl::ComparisonTag Tag>
auto numerical_comparison(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repo, auto lhs_numerical_ctor, auto rhs_numerical_ctor)
{
    auto data = ygg::Data<dl::Boolean<Uns, Tag>>(lhs_numerical_ctor.get_index(), rhs_numerical_ctor.get_index());
    return wrap_boolean(repo, repo.get_or_create(data).first);
}

template<dl::ComparisonTag Tag>
auto boolean_comparison(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repo, auto lhs_boolean_ctor, auto rhs_boolean_ctor)
{
    auto data = ygg::Data<dl::Boolean<Uns, Tag>>(lhs_boolean_ctor.get_index(), rhs_boolean_ctor.get_index());
    return wrap_boolean(repo, repo.get_or_create(data).first);
}

template<dl::NumericalBinaryTag Tag>
auto numerical_binary(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repo, auto lhs_numerical_ctor, auto rhs_numerical_ctor)
{
    auto data = ygg::Data<dl::Numerical<Uns, Tag>>(lhs_numerical_ctor.get_index(), rhs_numerical_ctor.get_index());
    return wrap_numerical(repo, repo.get_or_create(data).first);
}

template<dl::LogicalBinaryTag Tag>
auto logical_binary(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repo, auto lhs_boolean_ctor, auto rhs_boolean_ctor)
{
    auto data = ygg::Data<dl::Boolean<Uns, Tag>>(lhs_boolean_ctor.get_index(), rhs_boolean_ctor.get_index());
    return wrap_boolean(repo, repo.get_or_create(data).first);
}

auto logical_not(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repo, auto boolean_ctor)
{
    auto data = ygg::Data<dl::Boolean<Uns, dl::NotTag>>(boolean_ctor.get_index());
    return wrap_boolean(repo, repo.get_or_create(data).first);
}

}  // namespace

TEST(RunirTests, UnsFamilyComparisonsAndConstantsEvaluateAndFormat)
{
    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";

    auto parser = fp::Parser(domain);
    const auto planning_task = parser.parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search = datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    const auto state = search->state_repository->get_initial_state();

    auto dl_repository_factory = dl::ConstructorRepositoryFactoryFor<kr::UnsFamilyTag>();
    auto repository = dl_repository_factory.create(task->get_repository());
    auto& repo = *repository;

    // c_top -> n_count(c_top)
    auto top_data = ygg::Data<dl::Concept<Uns, dl::TopTag>>();
    auto top = repo.get_or_create(top_data).first;
    auto top_ctor_data = ygg::Data<dl::Constructor<Uns, dl::ConceptTag>>(top.get_index());
    auto top_ctor = repo.get_or_create(top_ctor_data).first;

    auto count_data = ygg::Data<dl::Numerical<Uns, dl::CountTag>>(top_ctor.get_index());
    auto count = repo.get_or_create(count_data).first;
    auto count_ctor = wrap_numerical(repo, count);

    // Evaluation context over the initial state.
    auto builder = sem::Builder();
    auto denotation_repository_factory = sem::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = sem::EvaluationContext<Uns, p::GroundTag>(state, builder, denotation_repository);

    // |c_top| is the number of objects; build n_const with exactly that value.
    const auto num_objects = sem::evaluate(count_ctor, context).get();
    EXPECT_GT(num_objects, 0u);

    auto n_const_eq = numerical_constant(repo, num_objects);
    auto n_const_zero = numerical_constant(repo, 0);

    // Numerical comparisons against |c_top|.
    EXPECT_TRUE(sem::evaluate(numerical_comparison<dl::EqTag<dl::NumericalTag>>(repo, count_ctor, n_const_eq), context).get());
    EXPECT_FALSE(sem::evaluate(numerical_comparison<dl::NeqTag<dl::NumericalTag>>(repo, count_ctor, n_const_eq), context).get());
    EXPECT_FALSE(sem::evaluate(numerical_comparison<dl::LtTag<dl::NumericalTag>>(repo, count_ctor, n_const_eq), context).get());
    EXPECT_TRUE(sem::evaluate(numerical_comparison<dl::LeTag<dl::NumericalTag>>(repo, count_ctor, n_const_eq), context).get());
    EXPECT_TRUE(sem::evaluate(numerical_comparison<dl::GtTag<dl::NumericalTag>>(repo, count_ctor, n_const_zero), context).get());
    EXPECT_TRUE(sem::evaluate(numerical_comparison<dl::GeTag<dl::NumericalTag>>(repo, count_ctor, n_const_eq), context).get());

    // Boolean constants and comparisons.
    auto b_true = boolean_constant(repo, true);
    auto b_false = boolean_constant(repo, false);
    EXPECT_TRUE(sem::evaluate(b_true, context).get());
    EXPECT_FALSE(sem::evaluate(b_false, context).get());
    EXPECT_TRUE(sem::evaluate(boolean_comparison<dl::EqTag<dl::BooleanTag>>(repo, b_true, b_true), context).get());
    EXPECT_FALSE(sem::evaluate(boolean_comparison<dl::EqTag<dl::BooleanTag>>(repo, b_true, b_false), context).get());
    EXPECT_TRUE(sem::evaluate(boolean_comparison<dl::NeqTag<dl::BooleanTag>>(repo, b_true, b_false), context).get());

    // Formatting round-trips the keywords and nested children.
    const auto lt = numerical_comparison<dl::LtTag<dl::NumericalTag>>(repo, count_ctor, n_const_zero);
    const auto formatted = fmt::format("{}", lt);
    EXPECT_NE(formatted.find("n_lt"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("n_count"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("c_top"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("n_const"), std::string::npos) << formatted;

    const auto b_eq = boolean_comparison<dl::EqTag<dl::BooleanTag>>(repo, b_true, b_false);
    const auto b_formatted = fmt::format("{}", b_eq);
    EXPECT_NE(b_formatted.find("b_eq"), std::string::npos) << b_formatted;
    EXPECT_NE(b_formatted.find("b_const"), std::string::npos) << b_formatted;
}

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

TEST(RunirTests, UnsFamilyArithmeticLogicalOperatorsEvaluateAndFormat)
{
    const auto domain = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";

    auto parser = fp::Parser(domain);
    const auto planning_task = parser.parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search = datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    const auto state = search->state_repository->get_initial_state();

    auto dl_repository_factory = dl::ConstructorRepositoryFactoryFor<kr::UnsFamilyTag>();
    auto repository = dl_repository_factory.create(task->get_repository());
    auto& repo = *repository;

    auto builder = sem::Builder();
    auto denotation_repository_factory = sem::DenotationRepositoryFactory();
    auto denotation_repository = denotation_repository_factory.create();
    auto context = sem::EvaluationContext<Uns, p::GroundTag>(state, builder, denotation_repository);

    constexpr auto inf = std::numeric_limits<ygg::uint_t>::max();
    auto two = numerical_constant(repo, 2);
    auto five = numerical_constant(repo, 5);
    auto zero = numerical_constant(repo, 0);

    // Arithmetic.
    EXPECT_EQ(sem::evaluate(numerical_binary<dl::AddTag>(repo, two, five), context).get(), 7u);
    EXPECT_EQ(sem::evaluate(numerical_binary<dl::SubTag>(repo, two, five), context).get(), 0u);  // saturates at 0
    EXPECT_EQ(sem::evaluate(numerical_binary<dl::SubTag>(repo, five, two), context).get(), 3u);
    EXPECT_EQ(sem::evaluate(numerical_binary<dl::MulTag>(repo, two, five), context).get(), 10u);
    EXPECT_EQ(sem::evaluate(numerical_binary<dl::DivTag>(repo, five, two), context).get(), 2u);
    EXPECT_EQ(sem::evaluate(numerical_binary<dl::DivTag>(repo, five, zero), context).get(), inf);  // div by zero -> inf
    EXPECT_EQ(sem::evaluate(numerical_binary<dl::MinTag>(repo, two, five), context).get(), 2u);
    EXPECT_EQ(sem::evaluate(numerical_binary<dl::MaxTag>(repo, two, five), context).get(), 5u);

    // Logical.
    auto b_true = boolean_constant(repo, true);
    auto b_false = boolean_constant(repo, false);
    EXPECT_FALSE(sem::evaluate(logical_binary<dl::AndTag>(repo, b_true, b_false), context).get());
    EXPECT_TRUE(sem::evaluate(logical_binary<dl::AndTag>(repo, b_true, b_true), context).get());
    EXPECT_TRUE(sem::evaluate(logical_binary<dl::OrTag>(repo, b_true, b_false), context).get());
    EXPECT_FALSE(sem::evaluate(logical_binary<dl::OrTag>(repo, b_false, b_false), context).get());
    EXPECT_TRUE(sem::evaluate(logical_not(repo, b_false), context).get());
    EXPECT_FALSE(sem::evaluate(logical_not(repo, b_true), context).get());

    // Formatting.
    const auto add = numerical_binary<dl::AddTag>(repo, two, five);
    const auto add_formatted = fmt::format("{}", add);
    EXPECT_NE(add_formatted.find("n_add"), std::string::npos) << add_formatted;

    const auto and_not = logical_binary<dl::AndTag>(repo, b_true, logical_not(repo, b_false));
    const auto and_formatted = fmt::format("{}", and_not);
    EXPECT_NE(and_formatted.find("b_and"), std::string::npos) << and_formatted;
    EXPECT_NE(and_formatted.find("b_not"), std::string::npos) << and_formatted;
}

}  // namespace runir::tests
