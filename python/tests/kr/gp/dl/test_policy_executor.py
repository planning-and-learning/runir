from pathlib import Path

from pyrunir.datasets import (
    GroundTaskSearchContext,
    StateGraphCostMode,
    annotate_ground_state_graph,
    generate_ground_state_graph,
)
from pyrunir.kr.dl.semantics import ConstructorRepositoryFactory
from pyrunir.kr.gp import (
    PolicyProofStatus,
    find_ground_solution,
    prove_ground_solution,
    syntactic_complexity,
)
from pyrunir.kr.gp import RepositoryFactory as PolicyRepositoryFactory
from pyrunir.kr.gp.dl import PolicyFactory, PolicySpecification, parse_policy
from pytyr.common import ExecutionContext
from pytyr.formalism.planning import Parser, ParserOptions
from pytyr.planning import SearchStatus
from pytyr.planning.lifted import GroundTaskInstantiationOptions, Task


def test_france_et_al_aaai2021_policy_executor_for_gripper_task():
    root = Path(__file__).resolve().parents[5]
    data_dir = root / "data" / "planning-benchmarks" / "tests" / "classical" / "gripper"

    parser_options = ParserOptions()
    parser = Parser(data_dir / "domain.pddl", parser_options)
    planning_task = parser.parse_task(data_dir / "test-1.pddl", parser_options)
    planning_domain = parser.get_domain()

    execution_context = ExecutionContext(1)
    lifted_task = Task(planning_task)
    ground_task = lifted_task.instantiate_ground_task(execution_context, GroundTaskInstantiationOptions()).task
    search_context = GroundTaskSearchContext(ground_task, execution_context)

    state_graph = generate_ground_state_graph(search_context)
    annotated_state_graph = annotate_ground_state_graph(search_context, state_graph, StateGraphCostMode.UNIT_COST)

    dl_repository = ConstructorRepositoryFactory().create(planning_domain)
    policy_repository = PolicyRepositoryFactory().create(dl_repository)
    empty_policy = PolicyFactory.create_empty(policy_repository)
    empty_policy_description = str(empty_policy)
    assert empty_policy_description == str(parse_policy(empty_policy_description, planning_domain, policy_repository))
    assert syntactic_complexity(empty_policy) == 0

    policy = PolicyFactory.create(
        PolicySpecification.GRIPPER_FRANCE_ET_AL_AAAI2021,
        planning_domain,
        policy_repository,
    )
    policy_description = str(policy)
    reparsed_policy = parse_policy(policy_description, planning_domain, policy_repository)
    assert str(reparsed_policy) == policy_description
    assert syntactic_complexity(policy) == 16

    proof_result = prove_ground_solution(annotated_state_graph, policy)
    assert proof_result.status == PolicyProofStatus.SUCCESS
    assert proof_result.is_successful()
    assert proof_result.deadend_transitions == []
    assert proof_result.open_states == []
    assert proof_result.cycle == []

    search_result = find_ground_solution(search_context, policy)
    assert search_result.status == SearchStatus.SOLVED
    assert search_result.plan is not None
