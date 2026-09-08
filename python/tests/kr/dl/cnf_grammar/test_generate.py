import pytest

from pypddl.formalism import ParserOptions
from pyyggdrasil.execution import ExecutionContext
from pytyr.formalism.planning import Parser
from pytyr.planning.lifted import GroundTaskInstantiationOptions, Task
from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext, LiftedTaskContext
from pyrunir.kr.dl import base
from pyrunir.kr.dl.base import cnf_grammar, semantics
from pyrunir.kr.dl.base.grammar import ConstructorRepositoryFactory


def _make_gripper_context(gripper_data_dir):
    parser_options = ParserOptions()
    parser = Parser(gripper_data_dir / "domain.pddl", parser_options)
    gripper_planning_domain = parser.get_domain()
    execution_context = ExecutionContext(1)
    task = Task(parser.parse_task(gripper_data_dir / "test-1.pddl", parser_options)).instantiate_ground_task(
        execution_context, GroundTaskInstantiationOptions()
    ).task
    search_context = GroundTaskSearchContext(task, execution_context)
    task_context = GroundTaskContext(DomainContext(gripper_planning_domain), search_context)
    return gripper_planning_domain, search_context, task_context


def _generate(grammar_description, states, planning_domain, task_context, max_syntactic_complexity):
    grammar_repository = ConstructorRepositoryFactory().create(planning_domain)
    grammar = base.parse_grammar(grammar_description, planning_domain.get_domain(), grammar_repository)
    cnf_repository = cnf_grammar.ConstructorRepositoryFactory().create(planning_domain)
    cnf = cnf_grammar.translate(grammar, cnf_repository)
    options = cnf_grammar.GenerateOptions()
    options.max_syntactic_complexity = max_syntactic_complexity
    return cnf_grammar.generate_ground(
        cnf, states, task_context.domain_context.base_repository.get_dl_repository(), task_context.dl_denotation_repository, options
    )


def _concept_vector(concept, states, task_context):
    result = []
    for state in states:
        context = semantics.GroundEvaluationContext(state, task_context.dl_builder, task_context.dl_denotation_repository)
        result.append(tuple(sorted(object_.get_name() for object_ in concept.evaluate(context))))
    return tuple(result)


def test_generate_ground_and_cached_evaluation(gripper_data_dir) -> None:
    gripper_planning_domain, search_context, task_context = _make_gripper_context(gripper_data_dir)
    state = search_context.state_repository.get_initial_state(search_context.axiom_evaluator)

    result = _generate(
        "((c_0 (c_top))(r_0 (r_universal))(b_0 (b_nonempty c_0))(n_0 (n_count c_0)))",
        [state],
        gripper_planning_domain,
        task_context,
        2,
    )

    assert result.statistics.num_generated == 4
    assert result.statistics.num_pruned == 0
    assert result.statistics.num_kept == 4

    cache = semantics.DenotationCaches()
    context = semantics.GroundEvaluationContext(state, task_context.dl_builder, task_context.dl_denotation_repository)
    assert len(list(result.concepts[0].evaluate(context, cache))) == 6
    assert len(list(result.roles[0].evaluate(context, cache))) == 36
    assert result.booleans[0].evaluate(context, cache).get() is True
    assert result.numericals[0].evaluate(context, cache).get() == 6


def test_generate_lifted_uses_domain_constructors_and_task_denotations(gripper_data_dir) -> None:
    parser = Parser(gripper_data_dir / "domain.pddl", ParserOptions())
    planning_domain = parser.get_domain()
    execution = ExecutionContext(1)
    task = Task(parser.parse_task(gripper_data_dir / "test-1.pddl", ParserOptions()))
    search = LiftedTaskSearchContext(task, execution)
    task_context = LiftedTaskContext(DomainContext(planning_domain), search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    output_repository = task_context.domain_context.base_repository.get_dl_repository()
    grammar_repository = ConstructorRepositoryFactory().create(planning_domain)
    grammar = base.parse_grammar(
        "((c_0 (c_top))(n_0 (n_count c_0)))", planning_domain.get_domain(), grammar_repository
    )
    cnf_repository = cnf_grammar.ConstructorRepositoryFactory().create(planning_domain)
    cnf = cnf_grammar.translate(grammar, cnf_repository)
    options = cnf_grammar.GenerateOptions()
    options.max_syntactic_complexity = 2

    denotation_repository = task_context.dl_denotation_repository
    result = cnf_grammar.generate_lifted(cnf, [state], output_repository, denotation_repository, options)
    context = semantics.LiftedEvaluationContext(state, task_context.dl_builder, task_context.dl_denotation_repository)
    assert len(list(result.concepts[0].evaluate(context))) == 6
    assert result.numericals[0].evaluate(context).get() == 6

    other_task = Task(parser.parse_task(gripper_data_dir / "test-1.pddl", ParserOptions()))
    other_search = LiftedTaskSearchContext(other_task, execution)
    other_state = other_search.state_repository.get_initial_state(other_search.axiom_evaluator)
    with pytest.raises(ValueError, match="states and denotation repository for the same planning task"):
        cnf_grammar.generate_lifted(cnf, [state, other_state], output_repository, denotation_repository, options)

    other_context = LiftedTaskContext(task_context.domain_context, other_search)
    with pytest.raises(ValueError, match="states and denotation repository for the same planning task"):
        cnf_grammar.generate_lifted(cnf, [state], output_repository, other_context.dl_denotation_repository, options)

    foreign_parser = Parser(gripper_data_dir / "domain.pddl", ParserOptions())
    foreign_task = Task(foreign_parser.parse_task(gripper_data_dir / "test-1.pddl", ParserOptions()))
    foreign_search = LiftedTaskSearchContext(foreign_task, execution)
    foreign_state = foreign_search.state_repository.get_initial_state(foreign_search.axiom_evaluator)
    with pytest.raises(ValueError, match="states and output repository for the same planning domain"):
        cnf_grammar.generate_lifted(cnf, [foreign_state], output_repository, denotation_repository, options)

    foreign_repository = semantics.ConstructorRepositoryFactory().create(foreign_parser.get_domain())
    with pytest.raises(ValueError, match="grammar and output repository for the same planning domain"):
        cnf_grammar.generate_lifted(cnf, [], foreign_repository, denotation_repository, options)


def test_distance_evaluation_handles_shortest_zero_and_infinity(gripper_data_dir) -> None:
    planning_domain, search_context, task_context = _make_gripper_context(gripper_data_dir)
    state = search_context.state_repository.get_initial_state(search_context.axiom_evaluator)

    result = _generate(
        '((c_1 (c_nominal "rooma"))(c_2 (c_nominal "roomb"))(c_3 (c_bot))(r_1 (r_universal))'
        "(n_0 (n_distance c_1 r_1 c_2))(n_0 (n_distance c_1 r_1 c_1))(n_0 (n_distance c_3 r_1 c_2)))",
        [state],
        planning_domain,
        task_context,
        4,
    )

    context = semantics.GroundEvaluationContext(state, task_context.dl_builder, task_context.dl_denotation_repository)
    values = [feature.evaluate(context).get() for feature in result.numericals]
    cache = semantics.DenotationCaches()
    cached_values = [feature.evaluate(context, cache).get() for feature in result.numericals]

    assert values == [1, 0, 2**32 - 1]
    assert cached_values == values


def test_generate_distinguishes_ordered_state_denotation_vectors(gripper_data_dir) -> None:
    planning_domain, search_context, task_context = _make_gripper_context(gripper_data_dir)
    initial_node = search_context.successor_generator.get_initial_node(
        search_context.state_repository, search_context.axiom_evaluator
    )
    move_to_roomb = next(
        successor
        for successor in search_context.successor_generator.get_labeled_successor_nodes(
            initial_node, search_context.state_repository, search_context.axiom_evaluator
        )
        if successor.label.get_relation().get_name() == "move"
        and [object_.get_name() for object_ in successor.label.get_objects()] == ["rooma", "roomb"]
    )
    states = [initial_node.get_state(), move_to_roomb.node.get_state()]

    result = _generate(
        '((c_0 (c_atomic_state "room"))(c_0 (c_atomic_state "at-robby"))(c_0 (c_not c_0))(c_0 (c_and c_0 c_0)))',
        states,
        planning_domain,
        task_context,
        4,
    )
    vectors = [_concept_vector(concept, states, task_context) for concept in result.concepts]

    assert vectors.count((("rooma",), ("roomb",))) == 1
    assert vectors.count((("roomb",), ("rooma",))) == 1


def test_generate_keeps_equal_denotations_in_distinct_nonterminals(gripper_data_dir) -> None:
    planning_domain, search_context, task_context = _make_gripper_context(gripper_data_dir)
    state = search_context.state_repository.get_initial_state(search_context.axiom_evaluator)

    result = _generate("((c_1 (c_top))(c_0 (c_and c_1 c_1)))", [state], planning_domain, task_context, 3)

    assert result.statistics.num_generated == 2
    assert result.statistics.num_pruned == 0
    assert result.statistics.num_kept == 2
    assert len(result.concepts) == 1
    assert result.concepts[0].syntactic_complexity() == 3


def test_generate_computes_substitution_closure_independent_of_rule_order(gripper_data_dir) -> None:
    planning_domain, search_context, task_context = _make_gripper_context(gripper_data_dir)
    state = search_context.state_repository.get_initial_state(search_context.axiom_evaluator)

    result = _generate("((c_0 (c_1))(c_1 (c_2))(c_2 (c_top)))", [state], planning_domain, task_context, 1)

    assert result.statistics.num_generated == 1
    assert result.statistics.num_pruned == 0
    assert result.statistics.num_kept == 1
    assert [str(concept) for concept in result.concepts] == ["(c_top)"]


def test_generate_deduplicates_commutative_candidates(gripper_data_dir) -> None:
    planning_domain, search_context, task_context = _make_gripper_context(gripper_data_dir)
    state = search_context.state_repository.get_initial_state(search_context.axiom_evaluator)

    result = _generate(
        '((c_0 (c_atomic_state "ball"))(c_0 (c_atomic_state "at-robby"))(c_0 (c_and c_0 c_0)))',
        [state],
        planning_domain,
        task_context,
        3,
    )

    assert len(result.concepts) == 3
    assert result.statistics.num_generated == 5
    assert result.statistics.num_pruned == 2
    assert result.statistics.num_kept == 3
    assert sum(concept.syntactic_complexity() == 3 for concept in result.concepts) == 1


def test_generate_terminates_substitution_closure_cycle_without_states(gripper_data_dir) -> None:
    planning_domain, _, task_context = _make_gripper_context(gripper_data_dir)

    result = _generate("((c_0 (c_1))(c_1 (c_2))(c_2 (c_0))(c_2 (c_top)))", [], planning_domain, task_context, 1)

    assert result.statistics.num_generated == 1
    assert result.statistics.num_pruned == 0
    assert result.statistics.num_kept == 1
    assert [str(concept) for concept in result.concepts] == ["(c_top)"]
