from pathlib import Path
import pytest

from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext
from pyrunir.kr.dl import ext as dl_ext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext import dl
from pyyggdrasil.execution import ExecutionContext
from pypddl.formalism import ParserOptions
from pytyr.formalism.planning import Parser
from pytyr.planning.lifted import GroundTaskInstantiationOptions, Task


def _blocksworld_data_dir():
    root = Path(__file__).resolve().parents[6]
    return root / "data" / "planning-benchmarks" / "profiling" / "htg" / "blocksworld-large-simple"


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
    return search_context, planning_domain, ground_task


def _repositories():
    planning_domain = Parser(_blocksworld_domain(), ParserOptions()).get_domain()
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(planning_domain)
    repository = ext.RepositoryFactory().create(dl_repository)
    return planning_domain, repository


def test_paper_module_factory_descriptions_parse_and_format_round_trip():
    planning_domain, repository = _repositories()
    descriptions = dl.ModuleFactory.create_bonet_et_al_icaps2024_descriptions()

    modules = dl.parse_modules(descriptions, planning_domain, repository)

    assert [module.get_name() for module in modules] == ["on", "on-table", "tower", "blocks"]
    assert len(modules[0].get_concept_features()) == 9
    assert len(modules[0].get_boolean_features()) == 2
    assert len(modules[0].get_numerical_features()) == 3
    assert len(modules[3].get_concept_features()) == 1
    assert len(modules[3].get_numerical_features()) == 1
    assert "(:symbol blocks)" in str(modules[3])
    memory_transitions = modules[3].get_memory_transitions()
    assert len(memory_transitions) > 0
    first_transition = memory_transitions[0]
    assert len(first_transition) > 0
    first_rule = first_transition[0].get_variant()
    assert first_rule.get_source() is not None
    assert first_rule.get_target() is not None

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

    with pytest.raises(RuntimeError, match="Error! Expecting:.*here"):
        dl.parse_module_program('(:program (:entry root)', planning_domain, repository)


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

    with pytest.raises(RuntimeError):
        dl.parse_module_program(f'(:program (:entry missing) {root})', planning_domain, repository)
    with pytest.raises(RuntimeError):
        dl.parse_module_program(f'(:program (:entry root) {root} {root})', planning_domain, repository)
    with pytest.raises(RuntimeError):
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

    with pytest.raises(RuntimeError, match="argument signature"):
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

    with pytest.raises(RuntimeError, match=r'entry module "missing" is not declared'):
        dl.parse_module_program(f'(:program (:entry missing) {root})', planning_domain, repository)

    with pytest.raises(RuntimeError, match=r'Unknown memory state "missing"'):
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

    with pytest.raises(RuntimeError, match=r'Unknown register "r1"'):
        dl.parse_module_program("""(:program
    (:entry bad-register)
    (:module
        (:symbol bad-register)
        (:arguments)
        (:registers)
        (:entry m0)
        (:memory m0 m1)
        (:features)
        (:rules
            (:rule
                (:symbol auto5)
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:load
                        (:conditions)
                        (:concept
                            (c_top)
                        )
                        (:register
                            (:concept r1)
                        )
                    )
                )
            )
        )
    )
)""", planning_domain, repository)

    with pytest.raises(RuntimeError, match=r'Unknown feature "missing"'):
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
    search_context, planning_domain, ground_task = _ground_context_and_domain()
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(ground_task)
    repository = ext.RepositoryFactory().create(dl_repository)

    program = dl.ModuleFactory.create_bonet_et_al_icaps2024_program(planning_domain, repository)

    search_options = ext.GroundModuleProgramSearchOptions()
    assert not hasattr(search_options, "siw_options")
    search_options.max_arity = 1

    search_result = ext.find_ground_solution(search_context, program, search_options)
    assert search_result.status == ext.ModuleProgramProofStatus.SUCCESS
    assert search_result.is_successful()
    assert search_result.final_state is not None
    assert search_result.plan is not None
    assert search_result.plan.get_length() == 4

    proof = ext.prove_ground_solution(search_context, program, search_options)
    assert proof.status == ext.ModuleProgramProofStatus.SUCCESS
    assert proof.is_successful()
    assert proof.graph.get_num_vertices() == 16
    assert proof.graph.get_num_edges() == 15
    vertex = next(iter(proof.graph.get_vertex_indices()))
    assert isinstance(proof.graph.get_successor_indices(vertex), list)
    edge = next(iter(proof.graph.get_edge_indices()))
    assert proof.graph.get_source(edge) in proof.graph.get_vertex_indices()
    assert proof.graph.get_target(edge) in proof.graph.get_vertex_indices()
    assert edge in proof.graph.get_out_edge_indices(proof.graph.get_source(edge))
    vertex_label = proof.graph.get_vertex_property(vertex)
    assert vertex_label.memory_state is not None
    assert len(vertex_label.concept_registers) > 0
    assert len(vertex_label.role_registers) > 0
    assert len(proof.deadend_transitions) == 0
    assert len(proof.open_states) == 0
    assert len(proof.cycle) == 0


def test_executor_reports_structured_failure_statuses_from_python():
    search_context, planning_domain, ground_task = _ground_context_and_domain()
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
    empty_proof = ext.prove_ground_solution(search_context, empty_program, options)
    assert empty_proof.status == ext.ModuleProgramProofStatus.FAILURE
    assert empty_proof.graph.get_num_vertices() == 1
    assert empty_proof.graph.get_num_edges() == 0
    assert len(empty_proof.deadend_transitions) == 0
    assert len(empty_proof.open_states) > 0
    assert len(empty_proof.cycle) == 0

    load_loop = dl.parse_module_program("""(:program
    (:entry load-loop)
    (:module
        (:symbol load-loop)
        (:arguments)
        (:registers
            (:concept r0)
        )
        (:entry source)
        (:memory source)
        (:features)
        (:rules
            (:rule
                (:symbol auto9)
                (:expression
                    (:source-memory source)
                    (:target-memory source)
                    (:load
                        (:conditions)
                        (:concept
                            (c_top)
                        )
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
    load_proof = ext.prove_ground_solution(search_context, load_loop, options)
    assert load_proof.status == ext.ModuleProgramProofStatus.FAILURE
    assert len(load_proof.deadend_transitions) == 0
    assert len(load_proof.cycle) > 0


    with pytest.raises(RuntimeError, match=r'Unknown action "missing-action"'):
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

    result = ext.prove_lifted_solution(search_context, program, options)

    assert result.status == ext.ModuleProgramProofStatus.FAILURE
    assert not result.is_successful()
