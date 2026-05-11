#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <runir/kr/dl/grammar/grammar_factory.hpp>
#include <string>
#include <tyr/formalism/planning/parser.hpp>

namespace
{

std::filesystem::path runir_root() { return std::filesystem::path(RUNIR_ROOT_DIR); }

}  // namespace

TEST(RunirTests, FranceEtAlAaai2021GrammarFactoryForGripperDomain)
{
    namespace grammar = runir::kr::dl::grammar;
    namespace fp = tyr::formalism::planning;

    const auto domain_filepath = runir_root() / "data" / "tests" / "classical" / "gripper" / "domain.pddl";
    const auto planning_domain = fp::Parser(domain_filepath).get_domain();
    const auto domain = planning_domain.get_domain();

    const auto description = grammar::GrammarFactory::create_description(grammar::GrammarSpecification::FRANCE_ET_AL_AAAI2021, domain);
    std::cout << description << std::endl;

    const auto expected = std::string {
        "[start_symbols]\n"
        "    concept ::= <concept_start>\n"
        "    role ::= <role_start>\n"
        "    boolean ::= <boolean_start>\n"
        "    numerical ::= <numerical_start>\n"
        "[grammar_rules]\n"
        "    <concept_bot> ::= @concept_bot\n"
        "    <concept_top> ::= @concept_top\n"
        "    <concept_nominal_rooma> ::= @concept_nominal \"rooma\"\n"
        "    <concept_nominal_roomb> ::= @concept_nominal \"roomb\"\n"
        "    <concept_atomic_state_object> ::= @concept_atomic_state \"object\"\n"
        "    <concept_atomic_goal_true_object> ::= @concept_atomic_goal \"object\" true\n"
        "    <concept_atomic_goal_false_object> ::= @concept_atomic_goal \"object\" false\n"
        "    <concept_atomic_state_number> ::= @concept_atomic_state \"number\"\n"
        "    <concept_atomic_goal_true_number> ::= @concept_atomic_goal \"number\" true\n"
        "    <concept_atomic_goal_false_number> ::= @concept_atomic_goal \"number\" false\n"
        "    <concept_atomic_state_room> ::= @concept_atomic_state \"room\"\n"
        "    <concept_atomic_goal_true_room> ::= @concept_atomic_goal \"room\" true\n"
        "    <concept_atomic_goal_false_room> ::= @concept_atomic_goal \"room\" false\n"
        "    <concept_atomic_state_ball> ::= @concept_atomic_state \"ball\"\n"
        "    <concept_atomic_goal_true_ball> ::= @concept_atomic_goal \"ball\" true\n"
        "    <concept_atomic_goal_false_ball> ::= @concept_atomic_goal \"ball\" false\n"
        "    <concept_atomic_state_gripper> ::= @concept_atomic_state \"gripper\"\n"
        "    <concept_atomic_goal_true_gripper> ::= @concept_atomic_goal \"gripper\" true\n"
        "    <concept_atomic_goal_false_gripper> ::= @concept_atomic_goal \"gripper\" false\n"
        "    <concept_atomic_state_at-robby> ::= @concept_atomic_state \"at-robby\"\n"
        "    <concept_atomic_goal_true_at-robby> ::= @concept_atomic_goal \"at-robby\" true\n"
        "    <concept_atomic_goal_false_at-robby> ::= @concept_atomic_goal \"at-robby\" false\n"
        "    <role_atomic_state_at> ::= @role_atomic_state \"at\"\n"
        "    <role_atomic_goal_true_at> ::= @role_atomic_goal \"at\" true\n"
        "    <role_atomic_goal_false_at> ::= @role_atomic_goal \"at\" false\n"
        "    <concept_atomic_state_free> ::= @concept_atomic_state \"free\"\n"
        "    <concept_atomic_goal_true_free> ::= @concept_atomic_goal \"free\" true\n"
        "    <concept_atomic_goal_false_free> ::= @concept_atomic_goal \"free\" false\n"
        "    <role_atomic_state_carry> ::= @role_atomic_state \"carry\"\n"
        "    <role_atomic_goal_true_carry> ::= @role_atomic_goal \"carry\" true\n"
        "    <role_atomic_goal_false_carry> ::= @role_atomic_goal \"carry\" false\n"
        "    <concept_intersection> ::= @concept_intersection <concept> <concept>\n"
        "    <concept_negation> ::= @concept_negation <concept>\n"
        "    <concept_existential_quantification> ::= @concept_existential_quantification <role> <concept>\n"
        "    <concept_value_restriction> ::= @concept_value_restriction <role> <concept>\n"
        "    <concept_role_value_map_equality> ::= @concept_role_value_map_equality <role> <role>\n"
        "    <role_transitive_closure> ::= @role_transitive_closure <role_primitive>\n"
        "    <role_inverse> ::= @role_inverse <role_primitive>\n"
        "    <boolean_nonempty_concept> ::= @boolean_nonempty <concept>\n"
        "    <boolean_nonempty_role> ::= @boolean_nonempty <role>\n"
        "    <numerical_count_concept> ::= @numerical_count <concept>\n"
        "    <numerical_count_role> ::= @numerical_count <role>\n"
        "    <numerical_distance> ::= @numerical_distance <concept> @role_restriction <role_primitive> <concept_primitive> <concept>\n"
        "    <numerical_distance> ::= @numerical_distance <concept> <role_primitive> <concept>\n"
        "    <concept_primitive> ::= <concept_bot> | <concept_top> | <concept_nominal_rooma> | <concept_nominal_roomb> | <concept_atomic_state_object> | "
        "<concept_atomic_goal_true_object> | <concept_atomic_goal_false_object> | <concept_atomic_state_number> | <concept_atomic_goal_true_number> | "
        "<concept_atomic_goal_false_number> | <concept_atomic_state_room> | <concept_atomic_goal_true_room> | <concept_atomic_goal_false_room> | "
        "<concept_atomic_state_ball> | <concept_atomic_goal_true_ball> | <concept_atomic_goal_false_ball> | <concept_atomic_state_gripper> | "
        "<concept_atomic_goal_true_gripper> | <concept_atomic_goal_false_gripper> | <concept_atomic_state_at-robby> | <concept_atomic_goal_true_at-robby> | "
        "<concept_atomic_goal_false_at-robby> | <concept_atomic_state_free> | <concept_atomic_goal_true_free> | <concept_atomic_goal_false_free>\n"
        "    <concept_start> ::= <concept>\n"
        "    <concept> ::= <concept_intersection> | <concept_negation> | <concept_existential_quantification> | <concept_value_restriction> | "
        "<concept_role_value_map_equality> | <concept_primitive>\n"
        "    <role_primitive> ::= <role_atomic_state_at> | <role_atomic_goal_true_at> | <role_atomic_goal_false_at> | <role_atomic_state_carry> | "
        "<role_atomic_goal_true_carry> | <role_atomic_goal_false_carry>\n"
        "    <role_start> ::= <role>\n"
        "    <role> ::= <role_transitive_closure> | <role_inverse> | <role_primitive>\n"
        "    <boolean_start> ::= <boolean>\n"
        "    <boolean> ::= <boolean_nonempty_concept> | <boolean_nonempty_role>\n"
        "    <numerical_start> ::= <numerical>\n"
        "    <numerical> ::= <numerical_count_concept> | <numerical_count_role> | <numerical_distance>\n"
    };

    EXPECT_EQ(description, expected);

    auto repository_factory = grammar::ConstructorRepositoryFactory();
    auto repository = repository_factory.create(planning_domain.get_repository());
    auto other_repository = repository_factory.create(planning_domain.get_repository());
    EXPECT_EQ(repository.get_index(), 0);
    EXPECT_EQ(other_repository.get_index(), 1);

    const auto grammar_view = grammar::GrammarFactory::create(grammar::GrammarSpecification::FRANCE_ET_AL_AAAI2021, domain, repository);

    EXPECT_EQ(grammar_view.get_context().template size<grammar::GrammarTag>(), 1);
    EXPECT_EQ(grammar_view.get_index(), tyr::Index<grammar::GrammarTag>(0));
}
