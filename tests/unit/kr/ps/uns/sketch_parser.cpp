#include <filesystem>
#include <gtest/gtest.h>
#include <runir/kr/ps/uns/dl/parser.hpp>
#include <runir/kr/ps/uns/repository.hpp>
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

std::filesystem::path benchmark_prefix() { return std::filesystem::path(RUNIR_ROOT_DIR) / "data" / "planning-benchmarks"; }

}  // namespace

TEST(RunirTests, UnsFamilySketchParserLowersComparisonAndConstantFeatures)
{
    const auto domain_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";

    auto parser = fp::Parser(domain_file);
    const auto planning_task = parser.parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;

    auto dl_repository_factory = kr::dl::uns::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::uns::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(task->get_repository());
    auto repository = repository_factory.create(dl_repository);

    const auto domain = task->get_domain().get_domain();

    // A sketch whose features exercise the new uns constructors:
    //  - b0: boolean comparison over two booleans
    //  - b1: numerical comparison (yields a boolean) using a numerical constant
    //  - n0: a nonnegative integer constant (numerical feature)
    const auto description = R"((:sketch
        (:features
            (:boolean (:symbol b0) (:description "") (:expression (b_eq (b_nonempty (c_top)) (b_const true))))
            (:boolean (:symbol b1) (:description "") (:expression (n_lt (n_count (c_top)) (n_const 3))))
            (:numerical (:symbol n0) (:description "") (:expression (n_const 5)))
        )
        (:rules
            (:rule (:symbol r0) (:description "")
                (:expression
                    (:conditions (:positive b0) (:greater_zero n0))
                    (:effects (:negative b0) (:decreases n0))
                )
            )
        )
    ))";

    EXPECT_NO_THROW({
        const auto sketch = kr::ps::uns::dl::parse_sketch(description, domain, *repository);
        (void) sketch;
    });
}

TEST(RunirTests, UnsFamilySketchParserRejectsMalformedComparison)
{
    const auto domain_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl";

    auto parser = fp::Parser(domain_file);
    const auto planning_task = parser.parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;

    auto dl_repository_factory = kr::dl::uns::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::uns::RepositoryFactory();
    auto dl_repository = dl_repository_factory.create(task->get_repository());
    auto repository = repository_factory.create(dl_repository);
    const auto domain = task->get_domain().get_domain();

    // n_lt needs two operands.
    const auto description = R"((:sketch
        (:features
            (:boolean (:symbol b0) (:description "") (:expression (n_lt (n_count (c_top)))))
        )
        (:rules)
    ))";

    EXPECT_ANY_THROW({
        const auto sketch = kr::ps::uns::dl::parse_sketch(description, domain, *repository);
        (void) sketch;
    });
}

}  // namespace runir::tests
