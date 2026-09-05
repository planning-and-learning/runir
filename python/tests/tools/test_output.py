import json

from fixture_utils import read_fixture
from pypddl.formalism import ParserOptions
from pyyggdrasil.execution import ExecutionContext
from pytyr.formalism.planning import Parser
from pytyr.planning import lifted
from pytyr import tools as tyr_tools
from pytyr.tools import format_state
from pytyr.tools.output import format_dictionaries as format_planning_dictionaries

from pyrunir.datasets import LiftedTaskSearchContext
from pyrunir.kr import DomainContext, LiftedTaskContext, uns
from pyrunir.kr.dl import ext as dl_ext, uns as dl_uns
from pyrunir.kr.dl.base.semantics import ConstructorRepositoryFactory
from pyrunir.kr.ps import base, ext
from pyrunir.kr.ps.base.dl import parse_sketch, structural_termination
from pyrunir.kr.ps.ext.dl import parse_module, parse_module_program
from pyrunir.kr.ps.ext.dl import structural_termination as module_structural_termination
from pyrunir.kr.uns.dl import parse_classifier
from pyrunir.tools.dictionaries import Dictionaries
from pyrunir.tools.output import (
    format_classifier,
    format_dictionaries,
    format_execution_state,
    format_module_policy_graph,
    format_module_program,
    format_policy_graph,
    format_sketch,
)


def _task() -> lifted.Task:
    parser = Parser("""(define (domain output) (:requirements :strips)
        (:predicates (start) (done))
        (:action finish :parameters () :precondition (start)
            :effect (and (not (start)) (done))))""", None, ParserOptions())
    return lifted.Task(parser.parse_task(
        "(define (problem output) (:domain output) (:init (start)) (:goal (done)))",
        None, ParserOptions(),
    ))


def test_same_symbol_features_keep_native_identity_and_share_stable_aliases():
    task = _task()
    domain = task.get_formalism_task().get_domain()
    repository = base.RepositoryFactory().create(ConstructorRepositoryFactory().create(domain))
    dictionaries = Dictionaries(task)
    sketches = [parse_sketch(f"""(:sketch
        (:features (:boolean (:symbol same) (:expression (b_nonempty ({concept})))))
        (:rules (:rule (:symbol advance) (:expression
            (:conditions (positive same)) (:effects (negative same))))))""", domain, repository)
        for concept in ("c_top", "c_bot")]

    first, second = [format_sketch(sketch, dictionaries) for sketch in sketches]
    assert first == format_sketch(sketches[0], dictionaries)
    first_id, = first["boolean_features"]
    second_id, = second["boolean_features"]
    assert first_id != second_id
    definitions = format_dictionaries(dictionaries)
    assert definitions["features"] == {
        first_id: {"symbol": "same", "expression": "(b_nonempty (c_top))"},
        second_id: {"symbol": "same", "expression": "(b_nonempty (c_bot))"},
    }
    assert all(definitions["rules"][alias]["symbol"] == "advance" for data in (first, second) for alias in data["rules"])
    assert all("(positive same)" in definition["expression"] for definition in definitions["rules"].values())
    assert json.loads(json.dumps(definitions)) == definitions


def test_module_program_registers_rules_and_referenced_memories():
    task = _task()
    domain = task.get_formalism_task().get_domain()
    repository = ext.RepositoryFactory().create(dl_ext.ConstructorRepositoryFactory().create(domain))
    program = parse_module_program(read_fixture("kr/ps/ext/dl/acyclic_calls.program"), domain, repository)
    dictionaries = Dictionaries(task)

    formatted = format_module_program(program, dictionaries)
    definitions = format_dictionaries(dictionaries)
    assert definitions["programs"][dictionaries.programs[program]] == formatted
    assert set(formatted["modules"]) == set(definitions["modules"])
    assert formatted["entry_module"] == dictionaries.modules[program.get_entry_module()]
    rule, = definitions["module_rules"].values()
    assert rule["symbol"] == "auto21"
    assert definitions["memories"][rule["source"]] == "m0"
    assert definitions["memories"][rule["target"]] == "m1"
    assert json.loads(json.dumps(definitions)) == definitions


def test_classifier_literals_reference_declared_features_with_polarity():
    task = _task()
    domain = task.get_formalism_task().get_domain()
    repository = uns.RepositoryFactory().create(dl_uns.ConstructorRepositoryFactory().create(domain))
    classifier = parse_classifier(read_fixture("kr/uns/positive.classifier"), domain, repository)
    dictionaries = Dictionaries(task)

    formatted = format_classifier(classifier, dictionaries)
    features = format_dictionaries(dictionaries)["features"]
    assert formatted["symbol"] == "c0"
    assert {features[alias]["symbol"] for alias in formatted["features"]} == {"some_ball", "no_object", "unused"}
    assert {
        frozenset((features[literal["feature"]]["symbol"], literal["polarity"]) for literal in clause)
        for clause in formatted["clauses"]
    } == {frozenset({("some_ball", True), ("no_object", False)}), frozenset({("no_object", True)})}
    assert json.loads(json.dumps(formatted)) == formatted


def test_planning_definitions_use_shared_tyr_dictionary_and_only_selected_states():
    task = _task()
    execution = ExecutionContext(1)
    repository = lifted.StateRepositoryFactory().create(task)
    evaluator = lifted.AxiomEvaluatorFactory().create(task, execution)
    generator = lifted.SuccessorGeneratorFactory().create(task, execution)
    initial = generator.get_initial_node(repository, evaluator)
    successor, = generator.get_labeled_successor_nodes(initial, repository, evaluator)
    dictionaries = Dictionaries(task)

    assert isinstance(dictionaries.planning, tyr_tools.Dictionaries)
    state = successor.node.get_state()
    facts = format_state(state, dictionaries.planning)
    assert format_state(state, dictionaries.planning) == facts
    definitions = format_dictionaries(dictionaries)["planning"]
    assert definitions == format_planning_dictionaries(dictionaries.planning)
    assert definitions["states"] == {dictionaries.planning.states[state]: facts}
    assert initial.get_state() not in dictionaries.planning.states
    atom, = definitions["fluent_atoms"]
    assert atom == {"id": facts["fluent"][0], "atom": "(done)"}
    assert json.loads(json.dumps(definitions)) == definitions


def test_execution_states_share_planning_state_but_preserve_call_stacks():
    task = _task()
    domain = task.get_formalism_task().get_domain()
    domain_context = DomainContext(domain)
    context = LiftedTaskContext(domain_context, LiftedTaskSearchContext(task, ExecutionContext(1)))
    program = parse_module_program(
        read_fixture("kr/ps/ext/dl/acyclic_calls.program"), domain, domain_context.ext_repository,
    )
    expander = ext.LiftedSuccessorExpander(context, program)
    initial = expander.initial_state()
    step, = expander.control_steps(initial)
    called = step.target
    assert initial.state == called.state
    assert initial.call_stack != called.call_stack
    dictionaries = Dictionaries(task)

    first = format_execution_state(initial, dictionaries)
    second = format_execution_state(called, dictionaries)
    definitions = format_dictionaries(dictionaries)
    assert first["state"] == second["state"]
    assert first["call_stack"] != second["call_stack"]
    assert len(definitions["planning"]["states"]) == 1
    assert len(definitions["execution_states"]) == 2
    assert definitions["execution_states"][dictionaries.execution_states[initial]] == first
    assert definitions["execution_states"][dictionaries.execution_states[called]] == second
    assert definitions["call_stacks"][second["call_stack"]]["caller"] is not None
    for state in definitions["execution_states"].values():
        assert state["state"] in definitions["planning"]["states"]
        assert state["program"] in definitions["programs"]
        assert state["call_stack"] in definitions["call_stacks"]
    for stack in definitions["call_stacks"].values():
        assert stack["module"] in definitions["modules"]
        assert stack["memory_state"] in definitions["memories"]
        assert stack["caller"] is None or stack["caller"] in definitions["call_stacks"]
    for module in definitions["modules"].values():
        assert module["entry_memory_state"] in definitions["memories"]
        assert set(module["memory_states"]) <= definitions["memories"].keys()
        assert set(module["rules"]) <= definitions["module_rules"].keys()
    assert set(definitions["programs"][first["program"]]["modules"]) == definitions["modules"].keys()
    for rule in definitions["module_rules"].values():
        assert rule["source"] in definitions["memories"] and rule["target"] in definitions["memories"]
    assert first == format_execution_state(expander.initial_state(), dictionaries)
    assert format_dictionaries(dictionaries) == definitions
    assert json.loads(json.dumps(definitions)) == definitions


def test_policy_graph_preserves_cycle_edges_and_boolean_feature_references():
    task = _task()
    domain = task.get_formalism_task().get_domain()
    repository = base.RepositoryFactory().create(ConstructorRepositoryFactory().create(domain))
    sketch = parse_sketch("""(:sketch
        (:features (:boolean (:symbol toggle) (:expression (b_nonempty (c_top)))))
        (:rules
            (:rule (:symbol set) (:expression
                (:conditions (negative toggle)) (:effects (positive toggle))))
            (:rule (:symbol clear) (:expression
                (:conditions (positive toggle)) (:effects (negative toggle))))))""", domain, repository)
    graph = structural_termination(sketch, use_incomplete_preprocessing=False).counterexample
    assert graph is not None
    dictionaries = Dictionaries(task)

    formatted = format_policy_graph(graph, sketch, dictionaries)
    definitions = format_dictionaries(dictionaries)
    feature, = definitions["features"]
    vertices = {vertex["index"]: vertex for vertex in formatted["vertices"]}
    assert set(vertices) == set(graph.get_vertex_indices())
    assert {vertex["boolean_values"][feature] for vertex in vertices.values()} == {True, False}
    assert all(vertex["numerical_values"] == {} for vertex in vertices.values())
    assert {edge["index"] for edge in formatted["edges"]} == set(graph.get_edge_indices())
    for edge in formatted["edges"]:
        assert (edge["source"], edge["target"]) == (graph.get_source(edge["index"]), graph.get_target(edge["index"]))
        source = vertices[edge["source"]]["boolean_values"][feature]
        target = vertices[edge["target"]]["boolean_values"][feature]
        assert source != target
        assert definitions["rules"][edge["rule"]]["symbol"] == ("set" if target else "clear")
    assert json.loads(json.dumps(formatted)) == formatted


def test_module_policy_graph_preserves_memory_cycle_and_numerical_feature_references():
    task = _task()
    domain = task.get_formalism_task().get_domain()
    repository = ext.RepositoryFactory().create(dl_ext.ConstructorRepositoryFactory().create(domain))
    module = parse_module("""(:module (:symbol loop) (:arguments) (:registers)
        (:entry q0) (:memory q0 q1)
        (:features (:numerical (:symbol count) (:expression (n_count (c_top)))))
        (:rules
            (:rule (:symbol forth) (:expression (:source-memory q0) (:target-memory q1)
                (:sketch (:conditions (greater_zero count)) (:effects (unchanged count)))))
            (:rule (:symbol back) (:expression (:source-memory q1) (:target-memory q0)
                (:sketch (:conditions) (:effects (unchanged count)))))))""", domain, repository)
    graph = module_structural_termination(module, use_incomplete_preprocessing=False).counterexample
    assert graph is not None
    dictionaries = Dictionaries(task)

    formatted = format_module_policy_graph(graph, module, dictionaries)
    definitions = format_dictionaries(dictionaries)
    feature, = definitions["features"]
    vertices = {vertex["index"]: vertex for vertex in formatted["vertices"]}
    assert set(vertices) == set(graph.get_vertex_indices())
    assert {definitions["memories"][vertex["memory_state"]] for vertex in vertices.values()} == {"q0", "q1"}
    assert all(vertex["numerical_values"] == {feature: True} and vertex["boolean_values"] == {} for vertex in vertices.values())
    assert {edge["index"] for edge in formatted["edges"]} == set(graph.get_edge_indices())
    for edge in formatted["edges"]:
        assert (edge["source"], edge["target"]) == (graph.get_source(edge["index"]), graph.get_target(edge["index"]))
        rule = definitions["module_rules"][edge["rule"]]
        assert rule["source"] == vertices[edge["source"]]["memory_state"]
        assert rule["target"] == vertices[edge["target"]]["memory_state"]
        assert rule["source"] != rule["target"]
    assert json.loads(json.dumps(formatted)) == formatted
