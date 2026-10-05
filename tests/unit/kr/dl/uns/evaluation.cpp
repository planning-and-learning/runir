#include "planning_fixtures.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <limits>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <string>

namespace runir::tests
{
namespace
{

namespace dl = runir::kr::dl;
namespace sem = runir::kr::dl::semantics;
using Uns = runir::kr::UnsFamilyTag;

auto wrap_boolean(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repository, auto boolean)
{
    auto data = ygg::Data<dl::Constructor<Uns, dl::BooleanTag>>(boolean.get_index());
    return repository.insert(data).first;
}

auto wrap_numerical(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repository, auto numerical)
{
    auto data = ygg::Data<dl::Constructor<Uns, dl::NumericalTag>>(numerical.get_index());
    return repository.insert(data).first;
}

auto boolean_constant(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repository, bool value)
{
    auto data = ygg::Data<dl::Boolean<Uns, dl::BooleanConstantTag>>(value);
    return wrap_boolean(repository, repository.insert(data).first);
}

auto numerical_constant(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repository, ygg::uint_t value)
{
    auto data = ygg::Data<dl::Numerical<Uns, dl::NumericalConstantTag>>(value);
    return wrap_numerical(repository, repository.insert(data).first);
}

template<dl::ComparisonTag Tag>
auto numerical_comparison(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repository, auto lhs, auto rhs)
{
    auto data = ygg::Data<dl::Boolean<Uns, Tag>>(lhs.get_index(), rhs.get_index());
    return wrap_boolean(repository, repository.insert(data).first);
}

template<dl::ComparisonTag Tag>
auto boolean_comparison(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repository, auto lhs, auto rhs)
{
    auto data = ygg::Data<dl::Boolean<Uns, Tag>>(lhs.get_index(), rhs.get_index());
    return wrap_boolean(repository, repository.insert(data).first);
}

template<dl::NumericalBinaryTag Tag>
auto numerical_binary(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repository, auto lhs, auto rhs)
{
    auto data = ygg::Data<dl::Numerical<Uns, Tag>>(lhs.get_index(), rhs.get_index());
    return wrap_numerical(repository, repository.insert(data).first);
}

template<dl::LogicalBinaryTag Tag>
auto logical_binary(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repository, auto lhs, auto rhs)
{
    auto data = ygg::Data<dl::Boolean<Uns, Tag>>(lhs.get_index(), rhs.get_index());
    return wrap_boolean(repository, repository.insert(data).first);
}

auto logical_not(dl::ConstructorRepositoryFor<kr::UnsFamilyTag>& repository, auto boolean)
{
    auto data = ygg::Data<dl::Boolean<Uns, dl::NotTag>>(boolean.get_index());
    return wrap_boolean(repository, repository.insert(data).first);
}

}  // namespace

TEST(RunirTests, UnsFamilyComparisonsAndConstantsEvaluateAndFormat)
{
    auto search = make_gripper_ground_context();
    const auto& task = search->task;
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);

    auto repository = dl::ConstructorRepositoryFactoryFor<kr::UnsFamilyTag>().create(task->get_repository());
    auto& repo = *repository;

    // c_top -> n_count(c_top)
    auto top_data = ygg::Data<dl::Concept<Uns, dl::TopTag>>();
    auto top = repo.insert(top_data).first;
    auto top_ctor_data = ygg::Data<dl::Constructor<Uns, dl::ConceptTag>>(top.get_index());
    auto top_ctor = repo.insert(top_ctor_data).first;

    auto count_data = ygg::Data<dl::Numerical<Uns, dl::CountTag>>(top_ctor.get_index());
    auto count = repo.insert(count_data).first;
    auto count_ctor = wrap_numerical(repo, count);

    // Evaluation context over the initial state.
    auto builder = sem::Builder();
    auto denotation_repository = sem::DenotationRepositoryFactory().create(task->get_repository());
    auto storage = sem::EvaluationStorage<Uns>(denotation_repository);
    auto context = sem::StateEvaluationContext<Uns, tyr::GroundTag>(state, builder, storage);

    // |c_top| is the number of objects; build n_const with exactly that value.
    const auto num_objects = sem::evaluate<tyr::GroundTag>(count_ctor, context).get();
    EXPECT_GT(num_objects, 0u);

    auto n_const_eq = numerical_constant(repo, num_objects);
    auto n_const_zero = numerical_constant(repo, 0);

    // Numerical comparisons against |c_top|.
    EXPECT_TRUE(sem::evaluate<tyr::GroundTag>(numerical_comparison<dl::EqTag<dl::NumericalTag>>(repo, count_ctor, n_const_eq), context).get());
    EXPECT_FALSE(sem::evaluate<tyr::GroundTag>(numerical_comparison<dl::NeqTag<dl::NumericalTag>>(repo, count_ctor, n_const_eq), context).get());
    EXPECT_FALSE(sem::evaluate<tyr::GroundTag>(numerical_comparison<dl::LtTag<dl::NumericalTag>>(repo, count_ctor, n_const_eq), context).get());
    EXPECT_TRUE(sem::evaluate<tyr::GroundTag>(numerical_comparison<dl::LeTag<dl::NumericalTag>>(repo, count_ctor, n_const_eq), context).get());
    EXPECT_TRUE(sem::evaluate<tyr::GroundTag>(numerical_comparison<dl::GtTag<dl::NumericalTag>>(repo, count_ctor, n_const_zero), context).get());
    EXPECT_TRUE(sem::evaluate<tyr::GroundTag>(numerical_comparison<dl::GeTag<dl::NumericalTag>>(repo, count_ctor, n_const_eq), context).get());

    // Boolean constants and comparisons.
    auto b_true = boolean_constant(repo, true);
    auto b_false = boolean_constant(repo, false);
    EXPECT_TRUE(sem::evaluate<tyr::GroundTag>(b_true, context).get());
    EXPECT_FALSE(sem::evaluate<tyr::GroundTag>(b_false, context).get());
    EXPECT_TRUE(sem::evaluate<tyr::GroundTag>(boolean_comparison<dl::EqTag<dl::BooleanTag>>(repo, b_true, b_true), context).get());
    EXPECT_FALSE(sem::evaluate<tyr::GroundTag>(boolean_comparison<dl::EqTag<dl::BooleanTag>>(repo, b_true, b_false), context).get());
    EXPECT_TRUE(sem::evaluate<tyr::GroundTag>(boolean_comparison<dl::NeqTag<dl::BooleanTag>>(repo, b_true, b_false), context).get());

    // Formatting round-trips the keywords and nested children.
    const auto lt = numerical_comparison<dl::LtTag<dl::NumericalTag>>(repo, count_ctor, n_const_zero);
    EXPECT_EQ(sem::syntactic_complexity(lt), 4);
    const auto formatted = fmt::format("{}", lt);
    EXPECT_NE(formatted.find("n_lt"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("n_count"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("c_top"), std::string::npos) << formatted;
    EXPECT_NE(formatted.find("n_const"), std::string::npos) << formatted;

    const auto b_eq = boolean_comparison<dl::EqTag<dl::BooleanTag>>(repo, b_true, b_false);
    EXPECT_EQ(sem::syntactic_complexity(b_eq), 3);
    const auto b_formatted = fmt::format("{}", b_eq);
    EXPECT_NE(b_formatted.find("b_eq"), std::string::npos) << b_formatted;
    EXPECT_NE(b_formatted.find("b_const"), std::string::npos) << b_formatted;
}

TEST(RunirTests, UnsFamilyArithmeticLogicalOperatorsEvaluateAndFormat)
{
    auto search = make_gripper_ground_context();
    const auto& task = search->task;
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);

    auto repository = dl::ConstructorRepositoryFactoryFor<kr::UnsFamilyTag>().create(task->get_repository());
    auto& repo = *repository;

    auto builder = sem::Builder();
    auto denotation_repository = sem::DenotationRepositoryFactory().create(task->get_repository());
    auto storage = sem::EvaluationStorage<Uns>(denotation_repository);
    auto context = sem::StateEvaluationContext<Uns, tyr::GroundTag>(state, builder, storage);

    constexpr auto inf = std::numeric_limits<ygg::uint_t>::max();
    auto two = numerical_constant(repo, 2);
    auto five = numerical_constant(repo, 5);
    auto zero = numerical_constant(repo, 0);

    // Arithmetic.
    EXPECT_EQ(sem::evaluate<tyr::GroundTag>(numerical_binary<dl::AddTag>(repo, two, five), context).get(), 7u);
    EXPECT_EQ(sem::evaluate<tyr::GroundTag>(numerical_binary<dl::SubTag>(repo, two, five), context).get(), 0u);  // saturates at 0
    EXPECT_EQ(sem::evaluate<tyr::GroundTag>(numerical_binary<dl::SubTag>(repo, five, two), context).get(), 3u);
    EXPECT_EQ(sem::evaluate<tyr::GroundTag>(numerical_binary<dl::MulTag>(repo, two, five), context).get(), 10u);
    EXPECT_EQ(sem::evaluate<tyr::GroundTag>(numerical_binary<dl::DivTag>(repo, five, two), context).get(), 2u);
    EXPECT_EQ(sem::evaluate<tyr::GroundTag>(numerical_binary<dl::DivTag>(repo, five, zero), context).get(), inf);  // div by zero -> inf
    EXPECT_EQ(sem::evaluate<tyr::GroundTag>(numerical_binary<dl::MinTag>(repo, two, five), context).get(), 2u);
    EXPECT_EQ(sem::evaluate<tyr::GroundTag>(numerical_binary<dl::MaxTag>(repo, two, five), context).get(), 5u);

    // Logical.
    auto b_true = boolean_constant(repo, true);
    auto b_false = boolean_constant(repo, false);
    EXPECT_FALSE(sem::evaluate<tyr::GroundTag>(logical_binary<dl::AndTag>(repo, b_true, b_false), context).get());
    EXPECT_TRUE(sem::evaluate<tyr::GroundTag>(logical_binary<dl::AndTag>(repo, b_true, b_true), context).get());
    EXPECT_TRUE(sem::evaluate<tyr::GroundTag>(logical_binary<dl::OrTag>(repo, b_true, b_false), context).get());
    EXPECT_FALSE(sem::evaluate<tyr::GroundTag>(logical_binary<dl::OrTag>(repo, b_false, b_false), context).get());
    EXPECT_TRUE(sem::evaluate<tyr::GroundTag>(logical_not(repo, b_false), context).get());
    EXPECT_FALSE(sem::evaluate<tyr::GroundTag>(logical_not(repo, b_true), context).get());

    // Formatting.
    const auto add = numerical_binary<dl::AddTag>(repo, two, five);
    EXPECT_EQ(sem::syntactic_complexity(add), 3);
    const auto add_formatted = fmt::format("{}", add);
    EXPECT_NE(add_formatted.find("n_add"), std::string::npos) << add_formatted;

    const auto and_not = logical_binary<dl::AndTag>(repo, b_true, logical_not(repo, b_false));
    EXPECT_EQ(sem::syntactic_complexity(and_not), 4);
    const auto and_formatted = fmt::format("{}", and_not);
    EXPECT_NE(and_formatted.find("b_and"), std::string::npos) << and_formatted;
    EXPECT_NE(and_formatted.find("b_not"), std::string::npos) << and_formatted;
}

}  // namespace runir::tests
