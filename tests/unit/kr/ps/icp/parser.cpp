#include "fixtures.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/errors.hpp>
#include <runir/kr/ps/icp/dl/parser.hpp>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <tyr/formalism/planning/parser.hpp>

namespace runir::tests
{

namespace
{

const std::string policy = R"((:module
  (:symbol visit)
  (:arguments)
  (:registers (:concept saved) (:role pair))
  (:entry start)
  (:memory start done)
  (:features
    (:concept (:symbol all) (:expression (c_top)))
    (:concept (:symbol selected) (:expression (c_register saved)))
    (:concept (:symbol empty) (:expression (c_bot)))
    (:role (:symbol edge) (:expression (r_register pair)))
    (:boolean (:symbol ready) (:expression (b_nonempty (q_concept X (c_top)))))
    (:numerical (:symbol size) (:expression (n_count (c_top))))
    (:query (:symbol choices) (:expression (q_concept X (c_register saved)))))
  (:rules
    (:rule (:symbol select) (:expression (:source-memory start) (:target-memory start)
      (:load (:conditions) (:concept all) (:register (:concept saved)) (:effects (positive ready)))
      (:load (:conditions) (:role edge) (:register (:role pair)))))
    (:rule (:symbol move) (:expression (:source-memory start) (:target-memory done)
      (:crule (:action "move") (:arguments from to)
        (:conditions (positive ready) (greater_zero size))
        (:xconditions (belongs (:argument from) (:concept all))
                      (not-belongs (:register (:concept saved)) (:concept empty)))
        (:xeffects (exit (:argument from) (:concept selected)))
        (:effects (unchanged size))))))
  (:reset-spo (:prec all selected) (:prec selected empty))
))";
std::string replaced(std::string value, const std::string& from, const std::string& to)
{
    const auto pos = value.find(from);
    if (pos == std::string::npos)
        throw std::logic_error("Missing test replacement.");
    value.replace(pos, from.size(), to);
    return value;
}

}

TEST(RunirTests, IcpParserRoundTripAndValidation)
{
    const auto planning = tyr::formalism::planning::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto constructors = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning.get_repository());
    auto repository = kr::ps::icp::RepositoryFactory().create(constructors);
    const auto parse = [&](const std::string& text) { return kr::ps::icp::dl::parse_module(text, planning.get_domain(), *repository); };
    const auto module = parse(policy);
    ASSERT_EQ(module.get_memory_transitions().size(), 2);
    EXPECT_EQ(module.get_memory_transitions().front().size(), 2);
    EXPECT_EQ(module.get_query_features().size(), 1);
    EXPECT_EQ(module.get_reset_pairs().size(), 3);
    // Enter progress is checked against the actual successor, not required as a literal.
    const auto rendered = fmt::format("{}", module);
    EXPECT_EQ(parse(rendered), module);
    EXPECT_NE(rendered.find(":effects"), std::string::npos);
    const auto program = kr::ps::icp::dl::parse_program("(:program (:entry visit) " + rendered + ")", planning.get_domain(), *repository);
    EXPECT_EQ(program.get_module(), module);

    for (const auto& invalid :
         { replaced(policy, "(:arguments)", "(:arguments (:concept input))"),
           replaced(policy, "(:reset-spo (:prec all selected) (:prec selected empty))", ""),
           replaced(policy, "(:prec selected empty)", "(:prec selected all)"),
           replaced(policy, "(:prec selected empty)", "(:prec all all)"),
           replaced(policy, "(:prec selected empty)", "(:prec selected size)"),
           replaced(policy, "(:arguments from to)", "(:arguments from from)"),
           replaced(policy, "(:arguments from to)", "(:arguments from)"),
           replaced(policy, "(:argument from)", "(:argument absent)"),
           replaced(policy, "(c_register saved)", "(c_register absent)"),
           replaced(policy, "(c_register saved)", "(c_argument 0)"),
           replaced(policy, "(:concept all))\n                      ", "(:concept size))\n                      "),
           replaced(policy, "(:concept saved) (:role pair)", "(:concept saved) (:concept a) (:concept b) (:concept c) (:concept d) (:role pair)") })
    {
        try
        {
            parse(invalid);
            FAIL() << "Expected invalid ICP input to fail:\n" << invalid;
        }
        catch (const kr::SemanticError& error)
        {
            EXPECT_TRUE(error.diagnostic().location.has_value()) << error.what();
        }
    }

    EXPECT_THROW(kr::ps::icp::dl::parse_program("(:program (:entry visit) " + rendered + rendered + ")", planning.get_domain(), *repository), kr::ParseError);
    EXPECT_THROW(kr::ps::icp::dl::parse_program("(:program (:entry absent) " + rendered + ")", planning.get_domain(), *repository), kr::UndefinedSymbolError);
}

}  // namespace runir::tests
