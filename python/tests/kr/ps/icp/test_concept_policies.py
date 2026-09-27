import gc
from datetime import timedelta

import pytest
from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pyyggdrasil.execution import ExecutionContext
from pyyggdrasil.serialization import Dictionaries
from pytyr.formalism.planning import Parser
from pytyr.planning import lifted

from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext, LiftedTaskContext
from pyrunir.kr.dl.base.semantics import ConceptDenotation
from pyrunir.kr.ps import icp
from pyrunir.serialization import register_table, serialize, table


PROGRAM = """(:program (:entry main)
  (:module (:symbol main) (:arguments) (:registers (:concept selected))
    (:entry start) (:memory start pick move drop done)
    (:features
      (:concept (:symbol candidates) (:expression
        (c_some (r_atomic_goal "at" true) (c_top))))
      (:concept (:symbol selected) (:expression (c_register selected)))
      (:concept (:symbol carried) (:expression
        (c_some (r_atomic_state "carry") (c_top))))
      (:concept (:symbol robot) (:expression (c_atomic_state "at-robby")))
      (:concept (:symbol delivered) (:expression
        (c_some (r_and (r_atomic_state "at") (r_atomic_goal "at" true)) (c_top))))
      (:numerical (:symbol selected_count) (:expression (n_count (c_register selected))))
      (:numerical (:symbol carried_count) (:expression
        (n_count (c_some (r_atomic_state "carry") (c_top)))))
      (:query (:symbol locations) (:expression (q_atomic_state "at" (ball room)))))
    (:rules
      (:rule (:symbol load) (:expression (:source-memory start) (:target-memory pick)
        (:load (:conditions) (:concept candidates) (:register (:concept selected))
          (:effects (increases selected_count)))))
      (:rule (:symbol pick) (:expression (:source-memory pick) (:target-memory move)
        (:crule (:action "pick") (:arguments ball room gripper) (:conditions)
          (:xconditions (belongs (:argument ball) (:concept selected)))
          (:xeffects (enter (:argument ball) (:concept carried)))
          (:effects (increases carried_count)))))
      (:rule (:symbol move) (:expression (:source-memory move) (:target-memory drop)
        (:crule (:action "move") (:arguments from to) (:conditions)
          (:xconditions) (:xeffects (enter (:argument to) (:concept robot))) (:effects))))
      (:rule (:symbol drop) (:expression (:source-memory drop) (:target-memory done)
        (:crule (:action "drop") (:arguments ball room gripper) (:conditions)
          (:xconditions (belongs (:argument ball) (:concept selected)))
          (:xeffects (enter (:argument ball) (:concept delivered)))
          (:effects (decreases carried_count))))))
    (:reset-spo)))
"""


def context_and_program(kind, source=PROGRAM):
    directory = data_root() / "classical" / "tests" / "gripper"
    parser = Parser(directory / "domain.pddl", ParserOptions())
    task = lifted.Task(parser.parse_task(directory / "test-1.pddl", ParserOptions()))
    execution = ExecutionContext(1)
    domain = DomainContext(parser.get_domain())
    if kind == "Ground":
        context = GroundTaskContext(domain, GroundTaskSearchContext(task.instantiate_ground_task(execution).task, execution))
    else:
        context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
    program = icp.dl.parse_program(source, parser.get_domain(), domain.icp_repository)
    return context, program


def collect_steps(expander, state):
    steps = []

    def emit(step):
        steps.append(step)
        return True

    assert expander.for_each_successor(state, icp.ProgramSearchStatistics(), emit, lambda: False)
    return steps


@pytest.mark.parametrize("kind", ["Ground", "Lifted"])
@pytest.mark.parametrize("universal", [False, True])
def test_search_and_round_trip(kind, universal):
    context, program = context_and_program(kind)
    domain = context.search_context.task.get_formalism_task().get_domain()
    assert icp.dl.parse_program(str(program), domain, context.domain_context.icp_repository) == program
    options = getattr(icp, f"{kind}ProgramSearchOptions")()
    options.universal = universal
    result = getattr(icp, f"find_{kind.lower()}_solution")(context, program, options)
    assert result.status == icp.ProgramProofStatus.SUCCESS
    assert result.is_successful()
    assert result.statistics.num_expanded >= 4
    assert result.statistics.num_generated >= 4
    assert not result.deadend_states and not result.open_states and not result.cycle
    assert (result.plan is None) == universal


@pytest.mark.parametrize("kind", ["Ground", "Lifted"])
def test_histories_evaluation_and_owned_views(kind):
    context, program = context_and_program(kind)
    expander = getattr(icp, f"{kind}SuccessorExpander")(context, program)
    search = context.search_context
    node = search.successor_generator.get_initial_node(search.state_repository, search.axiom_evaluator)
    initial = expander.initial_state(node.get_state())
    assert all(len(list(history)) == 0 for history in initial.histories.concepts)
    step, = collect_steps(expander, initial)
    loaded = step.target
    assert loaded.state == initial.state and loaded != initial
    assert loaded.memory_state.get_name() == "pick"
    environment = getattr(icp, f"{kind}EvaluationEnvironment")(context, program)
    dl_context = environment.make_dl_context(loaded)
    numericals = {feature.get_symbol(): feature for feature in loaded.module.get_numerical_features()}
    assert icp.evaluate(numericals["selected_count"], dl_context).get() == 1
    assert icp.evaluate(numericals["carried_count"], dl_context).get() == 0
    query, = loaded.module.get_query_features()
    relation = icp.evaluate(query, dl_context)
    assert relation.arity() == 2
    rows = {tuple(row) for row in relation}
    assert len(rows) == 2
    assert len({ball for ball, _ in rows}) == 2
    robot = next(feature for feature in loaded.module.get_concept_features() if feature.get_symbol() == "robot")
    assert {room for _, room in rows} == {int(obj.get_index()) for obj in icp.evaluate(robot, dl_context)}
    assert len(collect_steps(expander, loaded)) == 2
    histories = loaded.histories
    counts = [len(list(history)) for history in histories.concepts]
    assert sum(counts) == 1
    dictionaries = Dictionaries()
    state_type = getattr(icp, f"{kind}ProgramState")
    history_type = getattr(icp, f"{kind}Histories")
    register_table(dictionaries, state_type, "states", "s", fields=("histories",))
    register_table(dictionaries, history_type, "histories", "h")
    register_table(dictionaries, ConceptDenotation, "concepts", "c", project=lambda value: {"values": [obj.get_name() for obj in value]})
    assert serialize(dictionaries, initial) == "s0"
    assert serialize(dictionaries, loaded) == "s1"
    assert table(dictionaries, state_type) == [{"histories": "h0"}, {"histories": "h1"}]
    assert len(table(dictionaries, history_type)) == 2
    del context, program, environment, dl_context, expander, initial, loaded, step, relation, query, robot, numericals
    gc.collect()
    assert [len(list(history)) for history in histories.concepts] == counts


@pytest.mark.parametrize("kind", ["Ground", "Lifted"])
def test_resource_limits(kind):
    context, program = context_and_program(kind)
    options = getattr(icp, f"{kind}ProgramSearchOptions")()
    options.max_num_states = 0
    search = getattr(icp, f"find_{kind.lower()}_solution")
    assert search(context, program, options).status == icp.ProgramProofStatus.OUT_OF_STATES
    options.max_num_states = 100
    options.max_time = timedelta(0)
    assert search(context, program, options).status == icp.ProgramProofStatus.OUT_OF_TIME


def test_serialization_fields_and_program_projection():
    context, program = context_and_program("Lifted")
    assert "histories" in icp.LiftedProgramState.Fields.__members__
    assert "effects" in icp.ConceptLoadRule.Fields.__members__
    assert "xeffects" in icp.CruleRule.Fields.__members__
    dictionaries = Dictionaries()
    register_table(dictionaries, icp.Program, "programs", "p", project=lambda value: {"module": str(value.get_module())})
    assert serialize(dictionaries, program) == "p0"
    assert table(dictionaries, icp.Program) == [{"module": str(program.get_module())}]
    pick = next(
        variant.get_variant()
        for transition in program.get_module().get_memory_transitions()
        for variant in transition
        if variant.get_symbol() == "pick"
    )
    condition, = pick.get_xconditions()
    dictionaries = Dictionaries()
    register_table(dictionaries, icp.XCondition, "conditions", "x")
    register_table(dictionaries, icp.dl.ConceptFeature, "features", "f", fields=())
    assert serialize(dictionaries, condition) == "x0"
    assert table(dictionaries, icp.XCondition) == [{
        "operation": "belongs", "argument_position": 0, "register": None, "concept": "f0",
    }]
