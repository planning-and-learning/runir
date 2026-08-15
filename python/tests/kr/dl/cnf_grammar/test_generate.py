from pypddl.formalism import ParserOptions
from pyyggdrasil.execution import ExecutionContext
from pytyr.formalism.planning import Parser
from pytyr.planning.lifted import GroundTaskInstantiationOptions, Task
from pyrunir.datasets import GroundTaskSearchContext
from pyrunir.kr import GroundTaskContext
from pyrunir.kr.dl import base
from pyrunir.kr.dl.base import cnf_grammar, semantics
from pyrunir.kr.dl.base.grammar import ConstructorRepositoryFactory


def test_generate_ground_and_cached_evaluation(gripper_data_dir) -> None:
    parser_options = ParserOptions()
    parser = Parser(gripper_data_dir / "domain.pddl", parser_options)
    gripper_planning_domain = parser.get_domain()
    execution_context = ExecutionContext(1)
    task = Task(parser.parse_task(gripper_data_dir / "test-1.pddl", parser_options)).instantiate_ground_task(
        execution_context, GroundTaskInstantiationOptions()
    ).task
    search_context = GroundTaskSearchContext(task, execution_context)
    task_context = GroundTaskContext(search_context)
    state = search_context.state_repository.get_initial_state()

    grammar_repository = ConstructorRepositoryFactory().create(gripper_planning_domain)
    grammar = base.parse_grammar(
        "((c_0 (c_top))(r_0 (r_universal))(b_0 (b_nonempty c_0))(n_0 (n_count c_0)))",
        gripper_planning_domain.get_domain(),
        grammar_repository,
    )
    cnf_repository = cnf_grammar.ConstructorRepositoryFactory().create(gripper_planning_domain)
    cnf = cnf_grammar.translate(grammar, cnf_repository)
    options = cnf_grammar.GenerateOptions()
    options.max_syntactic_complexity = 2
    result = cnf_grammar.generate_ground(
        cnf,
        [state],
        task_context.base_dl_repository,
        options,
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
