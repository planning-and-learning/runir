#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <iostream>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <runir/kr/dl/grammar/grammar_factory.hpp>
#include <runir/kr/dl/grammar/parser.hpp>
#include <string>
#include <tyr/common/equal_to.hpp>
#include <tyr/formalism/planning/parser.hpp>

namespace
{

std::filesystem::path runir_root() { return std::filesystem::path(RUNIR_ROOT_DIR); }

std::filesystem::path benchmark_prefix() { return runir_root() / "data" / "planning-benchmarks"; }

}  // namespace

TEST(RunirTests, FranceEtAlAaai2021GrammarFactoryForGripperDomain)
{
    namespace grammar = runir::kr::dl::grammar;
    namespace fp = tyr::formalism::planning;

    const auto domain_filepath = benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto planning_domain = fp::Parser(domain_filepath).get_domain();
    const auto domain = planning_domain.get_domain();

    const auto expected = std::string { R"EXPECTED((grammar
  (:rules
    (c_3 (c_bot))
    (c_4 (c_top))
    (c_5 (c_nominal "rooma"))
    (c_6 (c_nominal "roomb"))
    (c_7 (c_atomic_state "object"))
    (c_8 (c_atomic_goal "object" true))
    (c_9 (c_atomic_goal "object" false))
    (c_10 (c_atomic_state "number"))
    (c_11 (c_atomic_goal "number" true))
    (c_12 (c_atomic_goal "number" false))
    (c_13 (c_atomic_state "room"))
    (c_14 (c_atomic_goal "room" true))
    (c_15 (c_atomic_goal "room" false))
    (c_16 (c_atomic_state "ball"))
    (c_17 (c_atomic_goal "ball" true))
    (c_18 (c_atomic_goal "ball" false))
    (c_19 (c_atomic_state "gripper"))
    (c_20 (c_atomic_goal "gripper" true))
    (c_21 (c_atomic_goal "gripper" false))
    (c_22 (c_atomic_state "at-robby"))
    (c_23 (c_atomic_goal "at-robby" true))
    (c_24 (c_atomic_goal "at-robby" false))
    (c_25 (c_atomic_state "free"))
    (c_26 (c_atomic_goal "free" true))
    (c_27 (c_atomic_goal "free" false))
    (c_28 (c_intersection c_1 c_1))
    (c_29 (c_negation c_1))
    (c_30 (c_existential_quantification r_1 c_1))
    (c_31 (c_value_restriction r_1 c_1))
    (c_32 (c_role_value_map_equality r_1 r_1))
    (c_2 (c_3 or c_4 or c_5 or c_6 or c_7 or c_8 or c_9 or c_10 or c_11 or c_12 or c_13 or c_14 or c_15 or c_16 or c_17 or c_18 or c_19 or c_20 or c_21 or c_22 or c_23 or c_24 or c_25 or c_26 or c_27))
    (c_0 (c_1))
    (c_1 (c_2 or c_28 or c_29 or c_30 or c_31 or c_32))
    (r_3 (r_atomic_state "at"))
    (r_4 (r_atomic_goal "at" true))
    (r_5 (r_atomic_goal "at" false))
    (r_6 (r_atomic_state "carry"))
    (r_7 (r_atomic_goal "carry" true))
    (r_8 (r_atomic_goal "carry" false))
    (r_9 (r_transitive_closure r_2))
    (r_10 (r_inverse r_2))
    (r_2 (r_3 or r_4 or r_5 or r_6 or r_7 or r_8))
    (r_0 (r_1))
    (r_1 (r_2 or r_9 or r_10))
    (b_2 (b_nonempty c_1))
    (b_3 (b_nonempty r_1))
    (b_0 (b_1))
    (b_1 (b_2 or b_3))
    (n_2 (n_count c_1))
    (n_3 (n_count r_1))
    (n_4 (n_distance c_1 (r_restriction r_2 c_2) c_1))
    (n_4 (n_distance c_1 r_2 c_1))
    (n_0 (n_1))
    (n_1 (n_2 or n_3 or n_4))
  )
)
)EXPECTED" };

    auto repository_factory = grammar::ConstructorRepositoryFactory();
    auto repository = repository_factory.create(planning_domain.get_repository());
    auto other_repository = repository_factory.create(planning_domain.get_repository());
    EXPECT_EQ(repository.get_index(), 0);
    EXPECT_EQ(other_repository.get_index(), 1);

    const auto grammar_view = grammar::GrammarFactory::create(grammar::GrammarSpecification::FRANCE_ET_AL_AAAI2021, domain, repository);
    const auto formatted = fmt::format("{}", grammar_view);
    std::cout << formatted << std::endl;
    EXPECT_EQ(formatted, expected);

    const auto reparsed_grammar_view = grammar::parse_grammar(formatted, domain, repository);
    EXPECT_TRUE(tyr::EqualTo<decltype(grammar_view)> {}(grammar_view, reparsed_grammar_view));

    EXPECT_EQ(grammar_view.get_context().template size<grammar::GrammarTag>(), 1);
    EXPECT_EQ(grammar_view.get_index(), tyr::Index<grammar::GrammarTag>(0));
}
