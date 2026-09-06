import json

from fixture_utils import read_fixture
from pytyr.planning import ground

from pyrunir.kr import DomainContext, GroundTaskContext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext.dl import parse_module_program
from pyrunir.serialization import Dictionaries, register_table, serialize, table


def test_runir_and_tyr_share_dictionary_references(ground_gripper_search_context):
    search = ground_gripper_search_context
    domain = search.task.get_formalism_task().get_domain()
    domain_context = DomainContext(domain)
    context = GroundTaskContext(domain_context, search)
    program = parse_module_program(
        read_fixture("kr/ps/ext/dl/acyclic_calls.program"), domain, domain_context.ext_repository,
    )
    expander = ext.GroundSuccessorExpander(context, program)
    initial = expander.initial_state()
    step, = expander.control_steps(initial)

    dictionaries = Dictionaries()
    dictionaries.register_table(ground.State, "states", "s")
    register_table(dictionaries, ext.GroundExecutionState, "execution_states", "e")
    register_table(dictionaries, ext.GroundCallStack, "call_stacks", "c")
    register_table(dictionaries, ext.ModuleProgram, "programs", "p")

    encoded_domain = dictionaries.serialize(domain)
    tyr_legends = dictionaries.enums()
    assert tyr_legends["Term"]
    assert dictionaries.serialize(initial.state) == "s0"
    assert serialize(dictionaries, initial) == "e0"
    assert serialize(dictionaries, step.target) == "e1"
    assert serialize(dictionaries, initial) == "e0"
    first, second = table(dictionaries, ext.GroundExecutionState)
    assert isinstance(first, dict) and isinstance(second, dict)
    assert first["state"] == second["state"] == "s0"
    assert first["call_stack"] != second["call_stack"]
    assert len(dictionaries.table(ground.State)) == 1
    assert dictionaries.tables()["execution_states"]["rows"] == [first, second]

    legends = dictionaries.enums()
    phase = next(entry for entry in legends["ExecutionPhase"] if entry["ref"] == first["phase"])
    assert phase["name"] == initial.phase.name
    assert legends["Ext.Rule"]
    assert all(legends[name] == rows for name, rows in tyr_legends.items())
    entries = [entry for rows in legends.values() for entry in rows]
    assert len({entry["ref"] for entry in entries}) == len(entries)
    encoded = json.dumps([encoded_domain, dictionaries.tables()])
    assert all(json.dumps(entry["ref"]) in encoded for entry in entries)
    assert dictionaries.serialize(domain) == encoded_domain
    assert serialize(dictionaries, initial) == "e0"
    assert dictionaries.enums() == legends
