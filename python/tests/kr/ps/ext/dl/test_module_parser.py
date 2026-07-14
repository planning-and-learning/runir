import gc

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
from pyrunir.kr.dl.uns import ConstructorRepositoryFactory as ClassifierDLRepositoryFactory
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext import dl
from pyrunir.kr.uns import RepositoryFactory as ClassifierRepositoryFactory
from pyrunir.kr.uns.dl import parse_classifier
from pyyggdrasil.execution import ExecutionContext
from pypddl.formalism import ParserOptions
from pytyr.formalism.planning import Parser
from pytyr.planning.lifted import GroundTaskInstantiationOptions, Task


def _blocksworld_data_dir():
    return data_root() / "classical" / "profiling" / "blocksworld-large-simple"


def _blocksworld_domain():
    return _blocksworld_data_dir() / "domain.pddl"


def _planning_task_and_domain():
    data_dir = _blocksworld_data_dir()
    parser = Parser(data_dir / "domain.pddl", ParserOptions())
    planning_task = parser.parse_task(data_dir / "p-100-2.pddl", ParserOptions())
    return planning_task, parser.get_domain()


def _ground_context_and_domain():
    planning_task, planning_domain = _planning_task_and_domain()
    execution_context = ExecutionContext(1)
    lifted_task = Task(planning_task)
    ground_task = lifted_task.instantiate_ground_task(execution_context, GroundTaskInstantiationOptions()).task
    search_context = GroundTaskSearchContext(ground_task, execution_context)
    return GroundTaskContext(search_context), planning_domain, ground_task


def _repositories():
    planning_domain = Parser(_blocksworld_domain(), ParserOptions()).get_domain()
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(planning_domain)
    repository = ext.RepositoryFactory().create(dl_repository)
    return planning_domain, repository


def _assert_diagnostic_at(error, source, marker):
    offset = source.index(marker)
    line_start = source.rfind("\n", 0, offset) + 1
    line_end = source.find("\n", offset)
    if line_end < 0:
        line_end = len(source)
    message = str(error)
    assert f"In line {source.count(chr(10), 0, offset) + 1}:" in message
    assert f"{source[line_start:line_end]}\n{'_' * (offset - line_start)}^_" in message


def test_paper_module_factory_descriptions_parse_and_format_round_trip():
    planning_domain, repository = _repositories()
    descriptions = dl.ModuleFactory.create_bonet_et_al_icaps2024_descriptions()

    modules = dl.parse_modules(descriptions, planning_domain, repository)

    assert [module.get_name() for module in modules] == ["on", "on-table", "tower", "blocks"]
    assert len(modules[0].get_concept_features()) == 12
    assert len(modules[0].get_boolean_features()) == 2
    assert len(modules[0].get_numerical_features()) == 3
    assert len(modules[3].get_concept_features()) == 2
    assert len(modules[3].get_numerical_features()) == 1
    assert "(:symbol blocks)" in str(modules[3])
    memory_transitions = modules[3].get_memory_transitions()
    assert len(memory_transitions) > 0
    first_transition = memory_transitions[0]
    assert len(first_transition) > 0
    first_rule = first_transition[0].get_variant()
    assert first_rule.get_source() is not None
    assert first_rule.get_target() is not None
    assert first_rule.get_feature().get_symbol() == "L_load"

    created_modules = dl.ModuleFactory.create_bonet_et_al_icaps2024_modules(planning_domain, repository)
    assert [module.get_name() for module in created_modules] == ["on", "on-table", "tower", "blocks"]
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
    assert program.get_entry_module().get_name() == "root"
    assert [module.get_name() for module in program.get_modules()] == ["root", "blocks", "tower", "on-table", "on"]

    formatted_program = str(program)
    assert "(:sketch\n                        (:conditions" in formatted_program

    reparsed_program = dl.parse_module_program(formatted_program, planning_domain, repository)
    assert reparsed_program.get_entry_module().get_name() == "root"
    assert len(reparsed_program.get_modules()) == 5


def test_module_program_parser_reports_x3_syntax_position():
    planning_domain, repository = _repositories()
    source = '(:program (:entry root)'

    with pytest.raises(ParseError, match="Error! Expecting:.*here") as raised:
        dl.parse_module_program(source, planning_domain, repository)

    assert "In line 1:" in str(raised.value)
    assert source in str(raised.value)
    assert "^_" in str(raised.value)


def test_module_parser_composes_dl_grammar_and_reports_symbolic_reference_position():
    planning_domain, repository = _repositories()
    source = """(:module
    (:symbol entry)
    (:arguments
        (:concept x)
    )
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features
        (:concept
            (:symbol bad)
            (
                ; The DL parser shares the outer source and skipper.
                :expression
                (c_argument missing)
            )
        )
    )
    (:rules)
)"""

    with pytest.raises(UndefinedSymbolError, match=r"Undefined concept argument: missing") as raised:
        dl.parse_module(source, planning_domain, repository)

    _assert_diagnostic_at(raised.value, source, "missing")


def test_module_parser_reports_malformed_nested_constructor_position():
    planning_domain, repository = _repositories()
    source = """(:module
    (:symbol entry)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features
        (:concept
            (:symbol bad)
            (:expression
                (c_not
                    (not_a_constructor)
                )
            )
        )
    )
    (:rules)
)"""

    with pytest.raises(ParseError) as raised:
        dl.parse_module(source, planning_domain, repository)

    _assert_diagnostic_at(raised.value, source, "(not_a_constructor)")


def test_load_rules_require_named_features_of_the_matching_category():
    planning_domain, repository = _repositories()

    def module(features, feature):
        return f"""(:module
    (:symbol load)
    (:arguments)
    (:registers (:concept r0))
    (:entry m0)
    (:memory m0 m1)
    (:features {features})
    (:rules
        (:rule
            (:symbol r0)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions)
                    (:concept {feature})
                    (:register (:concept r0))
                )
            )
        )
    )
)"""

    inline = module("", "(c_top)")
    with pytest.raises(ParseError) as raised:
        dl.parse_module(inline, planning_domain, repository)
    _assert_diagnostic_at(raised.value, inline, "(c_top)")

    wrong_category = module("(:role (:symbol shared) (:expression (r_universal)))", "shared ")
    with pytest.raises(UndefinedSymbolError, match=r"Undefined feature: shared") as raised:
        dl.parse_module(wrong_category, planning_domain, repository)
    _assert_diagnostic_at(raised.value, wrong_category, "shared )")


def test_module_parser_reports_fifth_register_position():
    planning_domain, repository = _repositories()
    source = """(:module
    (:symbol entry)
    (:arguments)
    (:registers
        (:concept r0)
        (:concept r1)
        (:concept r2)
        (:concept r3)
        (:concept r4)
    )
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
)"""

    with pytest.raises(InvalidExpressionError, match=r"supported maximum of 4") as raised:
        dl.parse_module(source, planning_domain, repository)

    _assert_diagnostic_at(raised.value, source, "(:concept r4)")


def test_parser_exception_hierarchy_matches_cpp_categories():
    assert issubclass(ParseError, SemanticError)
    assert issubclass(UndefinedSymbolError, SemanticError)
    assert issubclass(DuplicateDefinitionError, SemanticError)
    assert issubclass(ArityMismatchError, SemanticError)
    assert issubclass(InvalidExpressionError, SemanticError)
    assert not issubclass(SemanticError, RuntimeError)


def test_module_program_parser_rejects_invalid_wiring():
    planning_domain, repository = _repositories()
    root = """(:module
    (:symbol root)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
)"""

    with pytest.raises(UndefinedSymbolError):
        dl.parse_module_program(f'(:program (:entry missing) {root})', planning_domain, repository)
    with pytest.raises(DuplicateDefinitionError):
        dl.parse_module_program(f'(:program (:entry root) {root} {root})', planning_domain, repository)
    with pytest.raises(UndefinedSymbolError):
        dl.parse_module_program("""(:program
    (:entry root)
    (:module
        (:symbol root)
        (:arguments)
        (:registers)
        (:entry m0)
        (:memory m0 m1)
        (:features)
        (:rules
            (:rule
                (:symbol auto1)
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:call
                        (:conditions)
                        (:callee missing)
                        (:arguments)
                    )
                )
            )
        )
    )
)""", planning_domain, repository)

    with pytest.raises(InvalidExpressionError, match="argument signature"):
        dl.parse_module_program("""(:program
    (:entry caller)
    (:module
        (:symbol caller)
        (:arguments)
        (:registers)
        (:entry source)
        (:memory source target)
        (:features)
        (:rules
            (:rule
                (:symbol auto2)
                (:expression
                    (:source-memory source)
                    (:target-memory target)
                    (:call
                        (:conditions)
                        (:callee callee)
                        (:arguments)
                    )
                )
            )
        )
    )
    (:module
        (:symbol callee)
        (:arguments
            (:concept x)
        )
        (:registers)
        (:entry target)
        (:memory target)
        (:features)
        (:rules)
    )
)""", planning_domain, repository)

    with pytest.raises(UndefinedSymbolError, match=r"Undefined module: missing"):
        dl.parse_module_program(f'(:program (:entry missing) {root})', planning_domain, repository)

    with pytest.raises(UndefinedSymbolError, match=r"Undefined memory state: missing"):
        dl.parse_module_program("""(:program
    (:entry bad-memory)
    (:module
        (:symbol bad-memory)
        (:arguments)
        (:registers)
        (:entry m0)
        (:memory m0)
        (:features)
        (:rules
            (:rule
                (:symbol auto3)
                (:expression
                    (:source-memory m0)
                    (:target-memory missing)
                    (:sketch
                        (:conditions)
                        (:effects)
                    )
                )
            )
        )
    )
)""", planning_domain, repository)

    with pytest.raises(UndefinedSymbolError, match=r"Undefined register: r1"):
        dl.parse_module_program("""(:program
    (:entry bad-register)
    (:module
        (:symbol bad-register)
        (:arguments)
        (:registers)
        (:entry m0)
        (:memory m0 m1)
        (:features
            (:concept
                (:symbol B)
                (:expression (c_top))
            )
        )
        (:rules
            (:rule
                (:symbol auto5)
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:load
                        (:conditions)
                        (:concept B)
                        (:register
                            (:concept r1)
                        )
                    )
                )
            )
        )
    )
)""", planning_domain, repository)

    with pytest.raises(UndefinedSymbolError, match=r"Undefined feature: missing"):
        dl.parse_module_program("""(:program
    (:entry bad-feature)
    (:module
        (:symbol bad-feature)
        (:arguments)
        (:registers)
        (:entry m0)
        (:memory m0 m1)
        (:features)
        (:rules
            (:rule
                (:symbol auto7)
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:sketch
                        (:conditions
                            (greater_zero missing)
                        )
                        (:effects)
                    )
                )
            )
        )
    )
)""", planning_domain, repository)


def test_empty_module_factory_uses_ext_repositories():
    planning_domain, repository = _repositories()

    module = dl.ModuleFactory.create_empty(repository)
    reparsed = dl.parse_module(str(module), planning_domain, repository)

    assert module.get_name() == "empty"
    assert reparsed.get_name() == "empty"


def test_paper_modules_execute_on_small_blocksworld_instance_from_python():
    task_context, planning_domain, ground_task = _ground_context_and_domain()
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(ground_task)
    repository = ext.RepositoryFactory().create(dl_repository)

    program = dl.ModuleFactory.create_bonet_et_al_icaps2024_program(planning_domain, repository)

    search_options = ext.GroundModuleProgramSearchOptions()
    assert not hasattr(search_options, "brfs_options")
    assert not hasattr(search_options, "iw_options")
    assert not hasattr(search_options, "siw_options")
    assert not hasattr(search_options, "max_arity")
    assert search_options.max_num_states > 0
    assert search_options.max_time is None
    assert search_options.random_seed == 0
    assert search_options.shuffle_labeled_succ_nodes is False
    assert search_options.shuffle_choice_points is False
    assert search_options.universal is False
    assert search_options.classifier is None

    search_result = ext.find_ground_solution(task_context, program, search_options)
    assert search_result.status == ext.ModuleProgramProofStatus.SUCCESS
    assert search_result.is_successful()
    assert search_result.plan is not None
    assert search_result.plan.get_length() == 4

    classifier_dl_repository = ClassifierDLRepositoryFactory().create(ground_task)
    classifier_repository = ClassifierRepositoryFactory().create(classifier_dl_repository)
    classifier = parse_classifier(
        """(:classifier
    (:symbol all)
    (:features
        (:boolean
            (:symbol yes)
            (:expression (b_const true))
        )
    )
    (:expression (or (and yes)))
)""",
        planning_domain,
        classifier_repository,
    )
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


def test_executor_reports_structured_failure_statuses_from_python():
    task_context, planning_domain, ground_task = _ground_context_and_domain()
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(ground_task)
    repository = ext.RepositoryFactory().create(dl_repository)

    empty_program = dl.parse_module_program("""(:program
    (:entry empty)
    (:module
        (:symbol empty)
        (:arguments)
        (:registers)
        (:entry source)
        (:memory source)
        (:features)
        (:rules)
    )
)""", planning_domain, repository)
    options = ext.GroundModuleProgramSearchOptions()
    options.universal = True
    empty_proof = ext.find_ground_solution(task_context, empty_program, options)
    assert empty_proof.status == ext.ModuleProgramProofStatus.FAILURE
    assert empty_proof.graph.get_num_vertices() == 1
    assert empty_proof.graph.get_num_edges() == 0
    assert len(empty_proof.deadend_states) == 0
    assert len(empty_proof.open_states) > 0
    assert len(empty_proof.cycle) == 0

    load_deadend = dl.parse_module_program("""(:program
    (:entry load-deadend)
    (:module
        (:symbol load-deadend)
        (:arguments)
        (:registers
            (:concept r0)
        )
        (:entry source)
        (:memory source)
        (:features
            (:concept
                (:symbol B)
                (:expression (c_bot))
            )
        )
        (:rules
            (:rule
                (:symbol auto9)
                (:expression
                    (:source-memory source)
                    (:target-memory source)
                    (:load
                        (:conditions)
                        (:concept B)
                        (:register
                            (:concept r0)
                        )
                    )
                )
            )
        )
    )
)""", planning_domain, repository)
    options = ext.GroundModuleProgramSearchOptions()
    options.universal = True
    load_proof = ext.find_ground_solution(task_context, load_deadend, options)
    assert load_proof.status == ext.ModuleProgramProofStatus.FAILURE
    assert len(load_proof.deadend_states) == 1
    assert len(load_proof.open_states) == 0
    assert len(load_proof.cycle) == 0
    deadend_label = load_proof.graph.get_vertex_property(load_proof.deadend_states[0])
    assert deadend_label.is_unsolvable


    with pytest.raises(UndefinedSymbolError, match=r"Undefined action: missing-action"):
        dl.parse_module_program("""(:program
    (:entry no-action)
    (:module
        (:symbol no-action)
        (:arguments)
        (:registers)
        (:entry source)
        (:memory source target)
        (:features)
        (:rules
            (:rule
                (:symbol auto11)
                (:expression
                    (:source-memory source)
                    (:target-memory target)
                    (:do
                        (:conditions)
                        (:action "missing-action")
                        (:arguments)
                        (:effects)
                    )
                )
            )
        )
    )
)""", planning_domain, repository)


def test_lifted_executor_binding_reports_failure_status():
    planning_task, planning_domain = _planning_task_and_domain()
    execution_context = ExecutionContext(1)
    lifted_task = Task(planning_task)
    search_context = LiftedTaskSearchContext(lifted_task, execution_context)
    task_context = LiftedTaskContext(search_context)
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(lifted_task)
    repository = ext.RepositoryFactory().create(dl_repository)

    program = dl.parse_module_program("""(:program
    (:entry empty)
    (:module
        (:symbol empty)
        (:arguments)
        (:registers)
        (:entry source)
        (:memory source)
        (:features)
        (:rules)
    )
)""", planning_domain, repository)
    options = ext.LiftedModuleProgramSearchOptions()
    options.universal = True

    result = ext.find_lifted_solution(task_context, program, options)

    assert result.status == ext.ModuleProgramProofStatus.FAILURE
    assert not result.is_successful()


def test_ground_execution_views_own_their_task_and_program_contexts():
    task_context, planning_domain, ground_task = _ground_context_and_domain()
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(ground_task)
    repository = ext.RepositoryFactory().create(dl_repository)
    program = dl.ModuleFactory.create_bonet_et_al_icaps2024_program(planning_domain, repository)

    expander = ext.GroundSuccessorExpander(task_context, program)
    initial = expander.initial_state()
    duplicate_initial = expander.initial_state()
    assert isinstance(initial, ext.GroundExecutionState)
    assert initial.phase == ext.ExecutionPhase.EXTERNAL
    assert isinstance(initial.call_stack, ext.GroundCallStack)
    assert isinstance(initial.call_stack.registers, ext.GroundRegisterValues)
    assert isinstance(initial.call_stack.arguments, ext.GroundCallArguments)
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


def test_lifted_execution_views_and_proof_labels_survive_owner_destruction():
    planning_task, planning_domain = _planning_task_and_domain()
    execution_context = ExecutionContext(1)
    lifted_task = Task(planning_task)
    search_context = LiftedTaskSearchContext(lifted_task, execution_context)
    task_context = LiftedTaskContext(search_context)
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(lifted_task)
    repository = ext.RepositoryFactory().create(dl_repository)
    program = dl.parse_module_program("""(:program
    (:entry empty)
    (:module
        (:symbol empty)
        (:arguments)
        (:registers)
        (:entry source)
        (:memory source)
        (:features)
        (:rules)
    )
)""", planning_domain, repository)

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
