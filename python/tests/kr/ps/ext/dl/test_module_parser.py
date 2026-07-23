import gc
from pathlib import Path
from typing import TypedDict, cast

from fixture_utils import load_fixture, read_fixture
from pypddl_datasets import data_root
import pytest

from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext
from pyrunir.kr import (
    ArityMismatchError,
    DuplicateDefinitionError,
    GroundTaskContext,
    InvalidExpressionError,
    LiftedTaskContext,
    ParseError,
    SemanticError,
    UndefinedSymbolError,
)
from pyrunir.kr.dl import ext as dl_ext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext import dl
from pyrunir.kr.uns.dl import parse_classifier
from pyyggdrasil.execution import ExecutionContext
from pypddl.formalism import ParserOptions
from pytyr.formalism.planning import Parser, PlanningDomain, PlanningTask
from pytyr.planning.ground import Task as GroundTask
from pytyr.planning.lifted import GroundTaskInstantiationOptions, Task


class ExecutionFixture(TypedDict):
    name: str
    program_file: str
    universal: bool
    status: str
    num_vertices: int
    num_edges: int
    num_deadends: int
    has_open_states: bool
    cycle_length: int


class ModuleFactoryFixture(TypedDict):
    name: str
    concept_features: int
    role_features: int
    boolean_features: int
    numerical_features: int
    transitions: int
    syntactic_complexity: int


class ProgramFactoryFixture(TypedDict):
    entry: str
    modules: list[str]
    syntactic_complexity: int


_EXECUTION_SUITE = load_fixture("kr/ps/ext/execution.json")
EXECUTION_CASES = cast(list[ExecutionFixture], _EXECUTION_SUITE["cases"])
_MODULE_FACTORY_SUITE = load_fixture("kr/ps/ext/dl/module_factory.json")
MODULE_FACTORY_CASES = cast(list[ModuleFactoryFixture], _MODULE_FACTORY_SUITE["modules"])
PROGRAM_FACTORY_CASE = cast(ProgramFactoryFixture, _MODULE_FACTORY_SUITE["program"])
_FORMATTER_FRAGMENTS = load_fixture("kr/ps/ext/formatter_fragments.json")
MODULE_FACTORY_SKETCH_FRAGMENT = cast(str, _FORMATTER_FRAGMENTS["module_factory_sketch"])


def _blocksworld_data_dir() -> Path:
    return data_root() / "classical" / "profiling" / "blocksworld-large-simple"


def _blocksworld_domain() -> Path:
    return _blocksworld_data_dir() / "domain.pddl"


def _planning_task_and_domain() -> tuple[PlanningTask, PlanningDomain]:
    data_dir = _blocksworld_data_dir()
    parser = Parser(data_dir / "domain.pddl", ParserOptions())
    planning_task = parser.parse_task(data_dir / "p-100-2.pddl", ParserOptions())
    return planning_task, parser.get_domain()


def _ground_context_and_domain() -> tuple[GroundTaskContext, PlanningDomain, GroundTask]:
    planning_task, planning_domain = _planning_task_and_domain()
    execution_context = ExecutionContext(1)
    lifted_task = Task(planning_task)
    ground_task = lifted_task.instantiate_ground_task(execution_context, GroundTaskInstantiationOptions()).task
    search_context = GroundTaskSearchContext(ground_task, execution_context)
    return GroundTaskContext(search_context), planning_domain, ground_task


def _repositories() -> tuple[PlanningDomain, ext.Repository]:
    planning_domain = Parser(_blocksworld_domain(), ParserOptions()).get_domain()
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(planning_domain)
    repository = ext.RepositoryFactory().create(dl_repository)
    return planning_domain, repository


def _assert_diagnostic_at(error: SemanticError, source: str, marker: str) -> None:
    offset = source.index(marker)
    line_start = source.rfind("\n", 0, offset) + 1
    line_end = source.find("\n", offset)
    if line_end < 0:
        line_end = len(source)
    message = str(error)
    assert f"In line {source.count(chr(10), 0, offset) + 1}:" in message
    assert f"{source[line_start:line_end]}\n{'_' * (offset - line_start)}^_" in message


def test_paper_module_factory_descriptions_parse_and_format_round_trip() -> None:
    planning_domain, repository = _repositories()
    descriptions = dl.ModuleFactory.create_bonet_et_al_icaps2024_descriptions()

    modules = dl.parse_modules(descriptions, planning_domain, repository)

    assert [module.get_name() for module in modules] == [case["name"] for case in MODULE_FACTORY_CASES]
    assert len(modules) == len(MODULE_FACTORY_CASES)
    for module, case in zip(modules, MODULE_FACTORY_CASES):
        assert len(module.get_concept_features()) == case["concept_features"]
        assert len(module.get_role_features()) == case["role_features"]
        assert len(module.get_boolean_features()) == case["boolean_features"]
        assert len(module.get_numerical_features()) == case["numerical_features"]
        assert len(module.get_memory_transitions()) == case["transitions"]
        assert module.syntactic_complexity() == case["syntactic_complexity"]
    assert "(:symbol blocks)" in str(modules[3])
    for feature in (
        next(feature for module in modules for feature in module.get_concept_features()),
        next(feature for module in modules for feature in module.get_role_features()),
        next(feature for module in modules for feature in module.get_boolean_features()),
        next(feature for module in modules for feature in module.get_numerical_features()),
    ):
        assert feature.syntactic_complexity() == feature.get_variant().syntactic_complexity()
    for module in modules:
        feature_groups = (
            module.get_concept_features(),
            module.get_role_features(),
            module.get_boolean_features(),
            module.get_numerical_features(),
        )
        assert module.syntactic_complexity() == sum(
            feature.syntactic_complexity()
            for feature_group in feature_groups
            for feature in feature_group
        )
    memory_transitions = modules[3].get_memory_transitions()
    assert len(memory_transitions) > 0
    first_transition = memory_transitions[0]
    assert len(first_transition) > 0
    first_rule = first_transition[0].get_variant()
    assert isinstance(first_rule, (ext.ConceptLoadRule, ext.RoleLoadRule))
    assert first_rule.get_source() is not None
    assert first_rule.get_target() is not None
    assert first_rule.get_feature().get_symbol() == "L_load"

    created_modules = dl.ModuleFactory.create_bonet_et_al_icaps2024_modules(planning_domain, repository)
    assert [module.get_name() for module in created_modules] == [case["name"] for case in MODULE_FACTORY_CASES]
    assert dl.ModuleFactory.create(dl.ModuleSpecification.ON_BONET_ET_AL_ICAPS2024, planning_domain, repository).get_name() == "on"
    assert dl.ModuleFactory.create_on_bonet_et_al_icaps2024(planning_domain, repository).get_name() == "on"
    assert dl.ModuleFactory.create_on_table_bonet_et_al_icaps2024(planning_domain, repository).get_name() == "on-table"
    assert dl.ModuleFactory.create_tower_bonet_et_al_icaps2024(planning_domain, repository).get_name() == "tower"
    assert dl.ModuleFactory.create_blocks_bonet_et_al_icaps2024(planning_domain, repository).get_name() == "blocks"

    formatted_on = str(modules[0])
    reparsed_on = dl.parse_module(formatted_on, planning_domain, repository)

    assert reparsed_on.get_name() == "on"
    assert len(reparsed_on.get_concept_features()) == len(modules[0].get_concept_features())
    assert len(reparsed_on.get_boolean_features()) == len(modules[0].get_boolean_features())
    assert str(reparsed_on) == formatted_on

    program = dl.ModuleFactory.create_bonet_et_al_icaps2024_program(planning_domain, repository)
    assert program.get_entry_module().get_name() == PROGRAM_FACTORY_CASE["entry"]
    assert [module.get_name() for module in program.get_modules()] == PROGRAM_FACTORY_CASE["modules"]
    assert program.syntactic_complexity() == PROGRAM_FACTORY_CASE["syntactic_complexity"]

    formatted_program = str(program)
    assert MODULE_FACTORY_SKETCH_FRAGMENT in formatted_program

    reparsed_program = dl.parse_module_program(formatted_program, planning_domain, repository)
    assert reparsed_program.get_entry_module().get_name() == "root"
    assert len(reparsed_program.get_modules()) == 5


def test_module_program_parser_reports_x3_syntax_position() -> None:
    planning_domain, repository = _repositories()
    source = '(:program (:entry root)'

    with pytest.raises(ParseError, match="Error! Expecting:.*here") as raised:
        dl.parse_module_program(source, planning_domain, repository)

    assert "In line 1:" in str(raised.value)
    assert source in str(raised.value)
    assert "^_" in str(raised.value)


def test_module_parser_composes_dl_grammar_and_reports_symbolic_reference_position() -> None:
    planning_domain, repository = _repositories()
    source = read_fixture("kr/ps/ext/python/module_parser_composes_dl_grammar_and_reports_symbolic_reference_position/source.module")

    with pytest.raises(UndefinedSymbolError, match=r"Undefined concept argument: missing") as raised:
        dl.parse_module(source, planning_domain, repository)

    _assert_diagnostic_at(raised.value, source, "missing")


def test_module_parser_reports_malformed_nested_constructor_position() -> None:
    planning_domain, repository = _repositories()
    source = read_fixture("kr/ps/ext/python/module_parser_reports_malformed_nested_constructor_position/source.module")

    with pytest.raises(ParseError) as raised:
        dl.parse_module(source, planning_domain, repository)

    _assert_diagnostic_at(raised.value, source, "(not_a_constructor)")


def test_load_rules_require_named_features_of_the_matching_category() -> None:
    planning_domain, repository = _repositories()

    def module(features: str, feature: str) -> str:
        return read_fixture("kr/ps/ext/python/load_rules_require_named_features_of_the_matching_category/load.module").format(features=features, feature=feature)

    inline = module("", "(c_top)")
    with pytest.raises(ParseError) as inline_error:
        dl.parse_module(inline, planning_domain, repository)
    _assert_diagnostic_at(inline_error.value, inline, "(c_top)")

    wrong_category = module("(:role (:symbol shared) (:expression (r_universal)))", "shared ")
    with pytest.raises(UndefinedSymbolError, match=r"Undefined feature: shared") as category_error:
        dl.parse_module(wrong_category, planning_domain, repository)
    _assert_diagnostic_at(category_error.value, wrong_category, "shared )")


def test_module_parser_reports_fifth_register_position() -> None:
    planning_domain, repository = _repositories()
    source = read_fixture("kr/ps/ext/python/module_parser_reports_fifth_register_position/source.module")

    with pytest.raises(InvalidExpressionError, match=r"supported maximum of 4") as raised:
        dl.parse_module(source, planning_domain, repository)

    _assert_diagnostic_at(raised.value, source, "(:concept r4)")


def test_parser_exception_hierarchy_matches_cpp_categories() -> None:
    assert issubclass(ParseError, SemanticError)
    assert issubclass(UndefinedSymbolError, SemanticError)
    assert issubclass(DuplicateDefinitionError, SemanticError)
    assert issubclass(ArityMismatchError, SemanticError)
    assert issubclass(InvalidExpressionError, SemanticError)
    assert not issubclass(SemanticError, RuntimeError)


def test_module_program_parser_rejects_invalid_wiring() -> None:
    planning_domain, repository = _repositories()

    with pytest.raises(DuplicateDefinitionError):
        dl.parse_module_program(
            read_fixture("kr/ps/ext/executor/ext_module_program_parser_rejects_invalid_program_wiring/case_3.program"),
            planning_domain,
            repository,
        )
    with pytest.raises(UndefinedSymbolError):
        dl.parse_module_program(read_fixture("kr/ps/ext/python/module_program_parser_rejects_invalid_wiring/root.program"), planning_domain, repository)

    with pytest.raises(InvalidExpressionError, match="argument signature"):
        dl.parse_module_program(read_fixture("kr/ps/ext/python/module_program_parser_rejects_invalid_wiring/caller.program"), planning_domain, repository)

    with pytest.raises(UndefinedSymbolError, match=r"Undefined module: missing"):
        dl.parse_module_program(
            read_fixture("kr/ps/ext/executor/ext_module_program_parser_rejects_invalid_program_wiring/case_2.program"),
            planning_domain,
            repository,
        )

    with pytest.raises(UndefinedSymbolError, match=r"Undefined memory state: missing"):
        dl.parse_module_program(read_fixture("kr/ps/ext/python/module_program_parser_rejects_invalid_wiring/bad_memory.program"), planning_domain, repository)

    with pytest.raises(UndefinedSymbolError, match=r"Undefined register: r1"):
        dl.parse_module_program(read_fixture("kr/ps/ext/python/module_program_parser_rejects_invalid_wiring/bad_register.program"), planning_domain, repository)

    with pytest.raises(UndefinedSymbolError, match=r"Undefined feature: missing"):
        dl.parse_module_program(read_fixture("kr/ps/ext/python/module_program_parser_rejects_invalid_wiring/bad_feature.program"), planning_domain, repository)


def test_empty_module_factory_uses_ext_repositories() -> None:
    planning_domain, repository = _repositories()

    module = dl.ModuleFactory.create_empty(repository)
    reparsed = dl.parse_module(str(module), planning_domain, repository)

    assert module.get_name() == "empty"
    assert reparsed.get_name() == "empty"
    assert module.syntactic_complexity() == 0
    assert reparsed.syntactic_complexity() == 0


def test_paper_modules_execute_on_small_blocksworld_instance_from_python() -> None:
    task_context, planning_domain, _ground_task = _ground_context_and_domain()
    _dl_repository = task_context.ext_dl_repository
    repository = task_context.ext_repository

    program = dl.ModuleFactory.create_bonet_et_al_icaps2024_program(planning_domain, repository)

    search_options = ext.GroundModuleProgramSearchOptions()
    assert not hasattr(search_options, "brfs_options")
    assert not hasattr(search_options, "iw_options")
    assert not hasattr(search_options, "siw_options")
    assert not hasattr(search_options, "max_arity")
    assert search_options.max_num_states > 0
    assert search_options.max_time is None
    assert search_options.random_seed == 0
    assert search_options.shuffle_choice_points is False
    assert search_options.universal is False
    assert search_options.classifier is None

    search_result = ext.find_ground_solution(task_context, program, search_options)
    assert search_result.status == ext.ModuleProgramProofStatus.SUCCESS
    assert search_result.is_successful()
    assert search_result.plan is not None
    assert search_result.plan.get_length() == 4

    classifier_dl_repository = task_context.uns_dl_repository
    classifier_repository = task_context.uns_repository
    classifier = parse_classifier(read_fixture("kr/uns/always.classifier"), planning_domain, classifier_repository)
    classified_options = ext.GroundModuleProgramSearchOptions()
    classified_options.classifier = classifier
    lifted_options = ext.LiftedModuleProgramSearchOptions()
    assert lifted_options.classifier is None
    lifted_options.classifier = classifier
    assert lifted_options.classifier.get_index() == classifier.get_index()

    del classifier, classifier_repository, classifier_dl_repository
    gc.collect()
    classified_result = ext.find_ground_solution(task_context, program, classified_options)
    assert classified_result.status == ext.ModuleProgramProofStatus.FAILURE
    assert classified_result.graph.get_num_vertices() == 1
    assert classified_result.graph.get_num_edges() == 0
    assert len(classified_result.deadend_states) == 1
    assert classified_result.open_states == []
    classified_label = classified_result.graph.get_vertex_property(classified_result.deadend_states[0])
    assert not classified_label.is_goal
    assert not classified_label.is_alive
    assert classified_label.is_unsolvable

    proof_options = ext.GroundModuleProgramSearchOptions()
    proof_options.universal = True
    proof = ext.find_ground_solution(task_context, program, proof_options)
    assert proof.status == ext.ModuleProgramProofStatus.FAILURE
    assert not proof.is_successful()
    assert proof.graph.get_num_vertices() > search_result.graph.get_num_vertices()
    vertex = next(iter(proof.graph.get_vertex_indices()))
    assert isinstance(proof.graph.get_successor_indices(vertex), list)
    edge = next(iter(proof.graph.get_edge_indices()))
    assert proof.graph.get_source(edge) in proof.graph.get_vertex_indices()
    assert proof.graph.get_target(edge) in proof.graph.get_vertex_indices()
    assert edge in proof.graph.get_out_edge_indices(proof.graph.get_source(edge))
    vertex_label = proof.graph.get_vertex_property(vertex)
    assert vertex_label.execution_state.call_stack.memory_state is not None
    assert len(vertex_label.execution_state.call_stack.registers.concept_values) > 0
    assert len(vertex_label.execution_state.call_stack.registers.role_values) > 0
    assert len(proof.deadend_states) == 0
    assert len(proof.open_states) == 0
    assert len(proof.cycle) > 0


@pytest.mark.parametrize("case", EXECUTION_CASES, ids=[case["name"] for case in EXECUTION_CASES])
def test_executor_fixture(case: ExecutionFixture) -> None:
    task_context, planning_domain, _ground_task = _ground_context_and_domain()
    program = dl.parse_module_program(
        read_fixture(case["program_file"]), planning_domain, task_context.ext_repository
    )
    options = ext.GroundModuleProgramSearchOptions()
    options.universal = case["universal"]

    result = ext.find_ground_solution(task_context, program, options)

    expected_status = {
        "success": ext.ModuleProgramProofStatus.SUCCESS,
        "failure": ext.ModuleProgramProofStatus.FAILURE,
        "out_of_time": ext.ModuleProgramProofStatus.OUT_OF_TIME,
        "out_of_states": ext.ModuleProgramProofStatus.OUT_OF_STATES,
    }[case["status"]]
    assert result.status == expected_status
    assert result.graph.get_num_vertices() == case["num_vertices"]
    assert result.graph.get_num_edges() == case["num_edges"]
    assert len(result.deadend_states) == case["num_deadends"]
    assert bool(result.open_states) == case["has_open_states"]
    assert len(result.cycle) == case["cycle_length"]
    for vertex in result.deadend_states:
        assert result.graph.get_vertex_property(vertex).is_unsolvable


def test_module_program_parser_rejects_missing_action() -> None:
    task_context, planning_domain, _ground_task = _ground_context_and_domain()
    with pytest.raises(UndefinedSymbolError, match=r"Undefined action: missing-action"):
        dl.parse_module_program(
            read_fixture("kr/ps/ext/execution/missing_action.program"),
            planning_domain,
            task_context.ext_repository,
        )


def test_lifted_executor_binding_reports_failure_status() -> None:
    planning_task, planning_domain = _planning_task_and_domain()
    execution_context = ExecutionContext(1)
    lifted_task = Task(planning_task)
    search_context = LiftedTaskSearchContext(lifted_task, execution_context)
    task_context = LiftedTaskContext(search_context)
    _dl_repository = task_context.ext_dl_repository
    repository = task_context.ext_repository

    program = dl.parse_module_program(read_fixture("kr/ps/ext/execution/empty.program"), planning_domain, repository)
    options = ext.LiftedModuleProgramSearchOptions()
    options.universal = True

    result = ext.find_lifted_solution(task_context, program, options)

    assert result.status == ext.ModuleProgramProofStatus.FAILURE
    assert not result.is_successful()


def test_ground_execution_views_own_their_task_and_program_contexts() -> None:
    task_context, planning_domain, _ground_task = _ground_context_and_domain()
    dl_repository = task_context.ext_dl_repository
    repository = task_context.ext_repository
    program = dl.ModuleFactory.create_bonet_et_al_icaps2024_program(planning_domain, repository)

    expander = ext.GroundSuccessorExpander(task_context, program)
    initial = expander.initial_state()
    duplicate_initial = expander.initial_state()
    assert isinstance(initial, ext.GroundExecutionState)
    assert initial.phase == ext.ExecutionPhase.EXTERNAL
    assert isinstance(initial.call_stack, ext.GroundCallStack)
    assert isinstance(initial.call_stack.registers, ext.GroundRegisterValues)
    assert isinstance(initial.call_stack.arguments, ext.GroundCallArguments)
    assert initial.call_stack.arguments.concept_arguments == []
    assert initial == duplicate_initial
    assert initial.call_stack == duplicate_initial.call_stack
    assert initial.call_stack.registers == duplicate_initial.call_stack.registers
    assert initial.call_stack.arguments == duplicate_initial.call_stack.arguments
    assert len({initial, duplicate_initial}) == 1

    steps = expander.control_steps(initial)
    assert steps
    assert isinstance(steps[0], ext.GroundModuleProgramExecutionStep)
    target = steps[0].target

    del steps, duplicate_initial, initial, expander, program, repository, dl_repository, task_context
    gc.collect()

    assert target.state is not None
    assert target.program.get_entry_module() is not None
    assert target.call_stack.module is not None
    del target
    gc.collect()


def test_lifted_execution_views_and_proof_labels_survive_owner_destruction() -> None:
    planning_task, planning_domain = _planning_task_and_domain()
    execution_context = ExecutionContext(1)
    lifted_task = Task(planning_task)
    search_context = LiftedTaskSearchContext(lifted_task, execution_context)
    task_context = LiftedTaskContext(search_context)
    dl_repository = task_context.ext_dl_repository
    repository = task_context.ext_repository
    program = dl.parse_module_program(read_fixture("kr/ps/ext/execution/empty.program"), planning_domain, repository)

    expander = ext.LiftedSuccessorExpander(task_context, program)
    initial = expander.initial_state()
    assert isinstance(initial, ext.LiftedExecutionState)
    assert initial.phase == ext.ExecutionPhase.EXTERNAL
    assert isinstance(initial.call_stack, ext.LiftedCallStack)

    options = ext.LiftedModuleProgramSearchOptions()
    options.universal = True
    result = ext.find_lifted_solution(task_context, program, options)
    vertex = next(iter(result.graph.get_vertex_indices()))
    label = result.graph.get_vertex_property(vertex)
    assert isinstance(label, ext.LiftedModuleProgramProofVertexLabel)

    del result, expander, initial, program, repository, dl_repository, task_context, search_context, lifted_task
    gc.collect()

    assert label.execution_state.state is not None
    assert label.execution_state.call_stack.memory_state is not None
