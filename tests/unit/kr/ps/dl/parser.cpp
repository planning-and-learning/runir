#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <stdexcept>
#include <string>
#include <tyr/formalism/planning/parser.hpp>

namespace runir::tests
{
namespace
{

auto parse_gripper_task()
{
    const auto root = std::filesystem::path(BENCHMARKS_DIR) / "classical" / "tests" / "gripper";
    return tyr::formalism::planning::Parser(root / "domain.pddl").parse_task(root / "test-1.pddl");
}

template<typename Parse>
void expect_error_containing(Parse&& parse, const std::string& expected)
{
    try
    {
        parse();
        FAIL() << "Expected parsing to fail.";
    }
    catch (const std::runtime_error& error)
    {
        EXPECT_NE(std::string(error.what()).find(expected), std::string::npos) << error.what();
    }
}

}  // namespace

TEST(RunirTests, PolicyParsersRejectDuplicateSymbolsAndEntryArguments)
{
    const auto task = parse_gripper_task();
    const auto domain = task.get_domain().get_domain();

    auto base_dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(task.get_repository());
    auto base_repository = kr::ps::base::RepositoryFactory().create(base_dl_repository);

    expect_error_containing(
        [&]
        {
            kr::ps::base::dl::parse_sketch(R"((:sketch
                (:features
                    (:boolean (:symbol same) (:expression (b_nonempty (c_top))))
                    (:numerical (:symbol same) (:expression (n_count (c_top))))
                )
                (:rules)
            ))",
                                           domain,
                                           *base_repository);
        },
        "Duplicate feature symbol \"same\"");

    expect_error_containing(
        [&]
        {
            kr::ps::base::dl::parse_sketch(R"((:sketch
                (:features)
                (:rules
                    (:rule (:symbol same) (:expression (:conditions) (:effects)))
                    (:rule (:symbol same) (:expression (:conditions) (:effects)))
                )
            ))",
                                           domain,
                                           *base_repository);
        },
        "Duplicate rule symbol \"same\"");

    auto ext_dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(task.get_repository());
    auto ext_repository = kr::ps::ext::RepositoryFactory().create(ext_dl_repository);

    expect_error_containing(
        [&]
        {
            kr::ps::ext::dl::parse_module(R"((:module
                (:symbol root)
                (:arguments)
                (:registers)
                (:entry m0)
                (:memory m0)
                (:features
                    (:concept (:symbol same) (:expression (c_top)))
                    (:boolean (:symbol same) (:expression (b_nonempty (c_top))))
                )
                (:rules)
            ))",
                                          domain,
                                          *ext_repository);
        },
        "Duplicate feature symbol \"same\"");

    expect_error_containing(
        [&]
        {
            kr::ps::ext::dl::parse_module(R"((:module
                (:symbol root)
                (:arguments)
                (:registers)
                (:entry m0)
                (:memory m0)
                (:features)
                (:rules
                    (:rule
                        (:symbol same)
                        (:expression (:source-memory m0) (:target-memory m0) (:sketch (:conditions) (:effects)))
                    )
                    (:rule
                        (:symbol same)
                        (:expression (:source-memory m0) (:target-memory m0) (:sketch (:conditions) (:effects)))
                    )
                )
            ))",
                                          domain,
                                          *ext_repository);
        },
        "Duplicate rule symbol \"same\"");

    expect_error_containing(
        [&]
        {
            kr::ps::ext::dl::parse_module_program(R"((:program
                (:entry root)
                (:module
                    (:symbol root)
                    (:arguments (:concept input))
                    (:registers)
                    (:entry m0)
                    (:memory m0)
                    (:features)
                    (:rules)
                )
            ))",
                                                  domain,
                                                  *ext_repository);
        },
        "entry module \"root\" must not declare formal arguments");
}

TEST(RunirTests, ExtendedModuleFormatterPreservesAlternativeRuleGrouping)
{
    const auto task = parse_gripper_task();
    const auto domain = task.get_domain().get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(task.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);

    const auto module = kr::ps::ext::dl::parse_module(R"((:module
        (:symbol root)
        (:arguments)
        (:registers)
        (:entry m0)
        (:memory m0 m1)
        (:features
            (:boolean (:symbol H) (:expression (b_nonempty (c_top))))
        )
        (:rules
            (:rule
                (:symbol choose)
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:sketch (:conditions (positive H)) (:effects))
                    (:sketch (:conditions (negative H)) (:effects))
                )
            )
        )
    ))",
                                                      domain,
                                                      *repository);

    ASSERT_EQ(module.get_memory_transitions().size(), 1);
    ASSERT_EQ(module.get_memory_transitions().front().size(), 2);

    const auto formatted = fmt::format("{}", module);
    const auto symbol = std::string("(:symbol choose)");
    ASSERT_NE(formatted.find(symbol), std::string::npos) << formatted;
    EXPECT_EQ(formatted.find(symbol), formatted.rfind(symbol)) << formatted;

    const auto reparsed = kr::ps::ext::dl::parse_module(formatted, domain, *repository);
    ASSERT_EQ(reparsed.get_memory_transitions().size(), 1);
    EXPECT_EQ(reparsed.get_memory_transitions().front().size(), 2);
    EXPECT_EQ(fmt::format("{}", reparsed), formatted);
}

}  // namespace runir::tests
