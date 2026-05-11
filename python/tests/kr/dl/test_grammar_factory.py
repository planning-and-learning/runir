from pathlib import Path

from pyrunir.kr.dl.grammar import (
    ConstructorRepositoryFactory,
    GrammarFactory,
    GrammarSpecification,
)
from pytyr.formalism.planning import Parser, ParserOptions


def test_france_et_al_aaai2021_grammar_factory_for_gripper_domain():
    root = Path(__file__).resolve().parents[4]
    data_dir = root / "data" / "tests" / "classical" / "gripper"

    parser = Parser(data_dir / "domain.pddl", ParserOptions())
    planning_domain = parser.get_domain()
    domain = planning_domain.get_domain()

    description = GrammarFactory.create_description(GrammarSpecification.FRANCE_ET_AL_AAAI2021, domain)
    print(description)

    expected = """[start_symbols]
    concept ::= <concept_start>
    role ::= <role_start>
    boolean ::= <boolean_start>
    numerical ::= <numerical_start>
[grammar_rules]
    <concept_bot> ::= @concept_bot
    <concept_top> ::= @concept_top
    <concept_nominal_rooma> ::= @concept_nominal "rooma"
    <concept_nominal_roomb> ::= @concept_nominal "roomb"
    <concept_atomic_state_object> ::= @concept_atomic_state "object"
    <concept_atomic_goal_true_object> ::= @concept_atomic_goal "object" true
    <concept_atomic_goal_false_object> ::= @concept_atomic_goal "object" false
    <concept_atomic_state_number> ::= @concept_atomic_state "number"
    <concept_atomic_goal_true_number> ::= @concept_atomic_goal "number" true
    <concept_atomic_goal_false_number> ::= @concept_atomic_goal "number" false
    <concept_atomic_state_room> ::= @concept_atomic_state "room"
    <concept_atomic_goal_true_room> ::= @concept_atomic_goal "room" true
    <concept_atomic_goal_false_room> ::= @concept_atomic_goal "room" false
    <concept_atomic_state_ball> ::= @concept_atomic_state "ball"
    <concept_atomic_goal_true_ball> ::= @concept_atomic_goal "ball" true
    <concept_atomic_goal_false_ball> ::= @concept_atomic_goal "ball" false
    <concept_atomic_state_gripper> ::= @concept_atomic_state "gripper"
    <concept_atomic_goal_true_gripper> ::= @concept_atomic_goal "gripper" true
    <concept_atomic_goal_false_gripper> ::= @concept_atomic_goal "gripper" false
    <concept_atomic_state_at-robby> ::= @concept_atomic_state "at-robby"
    <concept_atomic_goal_true_at-robby> ::= @concept_atomic_goal "at-robby" true
    <concept_atomic_goal_false_at-robby> ::= @concept_atomic_goal "at-robby" false
    <role_atomic_state_at> ::= @role_atomic_state "at"
    <role_atomic_goal_true_at> ::= @role_atomic_goal "at" true
    <role_atomic_goal_false_at> ::= @role_atomic_goal "at" false
    <concept_atomic_state_free> ::= @concept_atomic_state "free"
    <concept_atomic_goal_true_free> ::= @concept_atomic_goal "free" true
    <concept_atomic_goal_false_free> ::= @concept_atomic_goal "free" false
    <role_atomic_state_carry> ::= @role_atomic_state "carry"
    <role_atomic_goal_true_carry> ::= @role_atomic_goal "carry" true
    <role_atomic_goal_false_carry> ::= @role_atomic_goal "carry" false
    <concept_intersection> ::= @concept_intersection <concept> <concept>
    <concept_negation> ::= @concept_negation <concept>
    <concept_existential_quantification> ::= @concept_existential_quantification <role> <concept>
    <concept_value_restriction> ::= @concept_value_restriction <role> <concept>
    <concept_role_value_map_equality> ::= @concept_role_value_map_equality <role> <role>
    <role_transitive_closure> ::= @role_transitive_closure <role_primitive>
    <role_inverse> ::= @role_inverse <role_primitive>
    <boolean_nonempty_concept> ::= @boolean_nonempty <concept>
    <boolean_nonempty_role> ::= @boolean_nonempty <role>
    <numerical_count_concept> ::= @numerical_count <concept>
    <numerical_count_role> ::= @numerical_count <role>
    <numerical_distance> ::= @numerical_distance <concept> @role_restriction <role_primitive> <concept_primitive> <concept>
    <numerical_distance> ::= @numerical_distance <concept> <role_primitive> <concept>
    <concept_primitive> ::= <concept_bot> | <concept_top> | <concept_nominal_rooma> | <concept_nominal_roomb> | <concept_atomic_state_object> | <concept_atomic_goal_true_object> | <concept_atomic_goal_false_object> | <concept_atomic_state_number> | <concept_atomic_goal_true_number> | <concept_atomic_goal_false_number> | <concept_atomic_state_room> | <concept_atomic_goal_true_room> | <concept_atomic_goal_false_room> | <concept_atomic_state_ball> | <concept_atomic_goal_true_ball> | <concept_atomic_goal_false_ball> | <concept_atomic_state_gripper> | <concept_atomic_goal_true_gripper> | <concept_atomic_goal_false_gripper> | <concept_atomic_state_at-robby> | <concept_atomic_goal_true_at-robby> | <concept_atomic_goal_false_at-robby> | <concept_atomic_state_free> | <concept_atomic_goal_true_free> | <concept_atomic_goal_false_free>
    <concept_start> ::= <concept>
    <concept> ::= <concept_intersection> | <concept_negation> | <concept_existential_quantification> | <concept_value_restriction> | <concept_role_value_map_equality> | <concept_primitive>
    <role_primitive> ::= <role_atomic_state_at> | <role_atomic_goal_true_at> | <role_atomic_goal_false_at> | <role_atomic_state_carry> | <role_atomic_goal_true_carry> | <role_atomic_goal_false_carry>
    <role_start> ::= <role>
    <role> ::= <role_transitive_closure> | <role_inverse> | <role_primitive>
    <boolean_start> ::= <boolean>
    <boolean> ::= <boolean_nonempty_concept> | <boolean_nonempty_role>
    <numerical_start> ::= <numerical>
    <numerical> ::= <numerical_count_concept> | <numerical_count_role> | <numerical_distance>
"""

    assert description == expected

    repository = ConstructorRepositoryFactory().create(planning_domain)
    grammar = GrammarFactory.create(GrammarSpecification.FRANCE_ET_AL_AAAI2021, domain, repository)

    assert grammar is not None
