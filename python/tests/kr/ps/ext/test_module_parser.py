from pathlib import Path

import pytest

try:
    from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext
    from pyrunir.kr.dl import ext as dl_ext
    from pyrunir.kr.ps import ext
except ModuleNotFoundError as exc:
    if "pyrunir._pyrunir" not in str(exc):
        raise
    import _pyrunir

    GroundTaskSearchContext = _pyrunir.datasets.GroundTaskSearchContext
    LiftedTaskSearchContext = _pyrunir.datasets.LiftedTaskSearchContext
    dl_ext = _pyrunir.kr.dl.ext
    ext = _pyrunir.kr.ps.ext
from pyyggdrasil import ExecutionContext
from pytyr.formalism.planning import Parser, ParserOptions
from pytyr.planning.lifted import GroundTaskInstantiationOptions, Task


def _blocksworld_data_dir():
    root = Path(__file__).resolve().parents[5]
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
    descriptions = ext.ModuleFactory.create_bonet_et_al_icaps2024_descriptions()

    modules = ext.parse_modules(descriptions, planning_domain, repository)

    assert [module.get_name() for module in modules] == ["on", "on-table", "tower", "blocks"]
    assert "(module \"blocks\"" in str(modules[3])
    memory_transitions = modules[3].get_memory_transitions()
    assert len(memory_transitions) > 0
    first_transition = memory_transitions[0]
    assert first_transition.get_source() is not None
    assert first_transition.get_target() is not None
    assert len(first_transition.get_rules()) > 0

    created_modules = ext.ModuleFactory.create_bonet_et_al_icaps2024_modules(planning_domain, repository)
    assert [module.get_name() for module in created_modules] == ["on", "on-table", "tower", "blocks"]
    assert ext.ModuleFactory.create(ext.ModuleSpecification.ON_BONET_ET_AL_ICAPS2024, planning_domain, repository).get_name() == "on"
    assert ext.ModuleFactory.create_on_bonet_et_al_icaps2024(planning_domain, repository).get_name() == "on"
    assert ext.ModuleFactory.create_on_table_bonet_et_al_icaps2024(planning_domain, repository).get_name() == "on-table"
    assert ext.ModuleFactory.create_tower_bonet_et_al_icaps2024(planning_domain, repository).get_name() == "tower"
    assert ext.ModuleFactory.create_blocks_bonet_et_al_icaps2024(planning_domain, repository).get_name() == "blocks"

    formatted_on = str(modules[0])
    reparsed_on = ext.parse_module(formatted_on, planning_domain, repository)

    assert reparsed_on.get_name() == "on"
    assert str(reparsed_on) == formatted_on

    program = ext.ModuleFactory.create_bonet_et_al_icaps2024_program(planning_domain, repository)
    assert program.get_entry_module().get_name() == "root"
    assert [module.get_name() for module in program.get_modules()] == ["root", "blocks", "tower", "on-table", "on"]

    formatted_program = str(program)
    reparsed_program = ext.parse_module_program(formatted_program, planning_domain, repository)
    assert reparsed_program.get_entry_module().get_name() == "root"
    assert len(reparsed_program.get_modules()) == 5


def test_module_program_parser_reports_x3_syntax_position():
    planning_domain, repository = _repositories()

    with pytest.raises(RuntimeError, match="Error! Expecting:.*here"):
        ext.parse_module_program('(program (:entry "root")', planning_domain, repository)


def test_module_program_parser_rejects_invalid_wiring():
    planning_domain, repository = _repositories()
    root = """(module "root"
      (:arguments)
      (:registers)
      (:entry "m0")
      (:memory "m0")
      (:features)
      (:transitions)
    )"""

    with pytest.raises(RuntimeError):
        ext.parse_module_program(f'(program (:entry "missing") {root})', planning_domain, repository)
    with pytest.raises(RuntimeError):
        ext.parse_module_program(f'(program (:entry "root") {root} {root})', planning_domain, repository)
    with pytest.raises(RuntimeError):
        ext.parse_module_program("""(program
          (:entry "root")
          (module "root"
            (:arguments)
            (:registers)
            (:entry "m0")
            (:memory "m0" "m1")
            (:features)
            (:transitions
              ("m0" "m1" (call (:conditions) (:callee "missing") (:arguments)))
            )
          )
        )""", planning_domain, repository)

    with pytest.raises(RuntimeError, match="argument signature.*offset"):
        ext.parse_module_program("""(program
          (:entry "caller")
          (module "caller"
            (:arguments)
            (:registers)
            (:entry "source")
            (:memory "source" "target")
            (:features)
            (:transitions
              ("source" "target" (call (:conditions) (:callee "callee") (:arguments)))
            )
          )
          (module "callee"
            (:arguments
              (concept "x" 0)
            )
            (:registers)
            (:entry "target")
            (:memory "target")
            (:features)
            (:transitions)
          )
        )""", planning_domain, repository)

    with pytest.raises(RuntimeError, match=r'entry module "missing" is not declared.*offset'):
        ext.parse_module_program(f'(program (:entry "missing") {root})', planning_domain, repository)

    with pytest.raises(RuntimeError, match=r'Unknown memory state "missing".*offset'):
        ext.parse_module_program("""(program
          (:entry "bad-memory")
          (module "bad-memory"
            (:arguments)
            (:registers)
            (:entry "m0")
            (:memory "m0")
            (:features)
            (:transitions
              ("m0" "missing" (sketch (:conditions) (:effects)))
            )
          )
        )""", planning_domain, repository)

    with pytest.raises(RuntimeError, match=r'Unknown register "missing".*offset'):
        ext.parse_module_program("""(program
          (:entry "bad-register")
          (module "bad-register"
            (:arguments)
            (:registers)
            (:entry "m0")
            (:memory "m0" "m1")
            (:features)
            (:transitions
              ("m0" "m1" (load (:conditions) (:concept (c_top)) (:register "missing")))
            )
          )
        )""", planning_domain, repository)

    with pytest.raises(RuntimeError, match=r'Unknown feature "missing".*offset'):
        ext.parse_module_program("""(program
          (:entry "bad-feature")
          (module "bad-feature"
            (:arguments)
            (:registers)
            (:entry "m0")
            (:memory "m0" "m1")
            (:features)
            (:transitions
              ("m0" "m1" (sketch (:conditions (greater_zero missing)) (:effects)))
            )
          )
        )""", planning_domain, repository)


def test_empty_module_factory_uses_ext_repositories():
    planning_domain, repository = _repositories()

    module = ext.ModuleFactory.create_empty(repository)
    reparsed = ext.parse_module(str(module), planning_domain, repository)

    assert module.get_name() == "empty"
    assert reparsed.get_name() == "empty"


def test_paper_modules_execute_on_small_blocksworld_instance_from_python():
    search_context, planning_domain, ground_task = _ground_context_and_domain()
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(ground_task)
    repository = ext.RepositoryFactory().create(dl_repository)

    program = ext.ModuleFactory.create_bonet_et_al_icaps2024_program(planning_domain, repository)

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

    empty_program = ext.parse_module_program("""(program
      (:entry "empty")
      (module "empty"
        (:arguments)
        (:registers)
        (:entry "source")
        (:memory "source")
        (:features)
        (:transitions)
      )
    )""", planning_domain, repository)
    options = ext.GroundModuleProgramSearchOptions()
    empty_proof = ext.prove_ground_solution(search_context, empty_program, options)
    assert empty_proof.status == ext.ModuleProgramProofStatus.FAILURE
    assert empty_proof.graph.get_num_vertices() == 1
    assert empty_proof.graph.get_num_edges() == 1
    assert len(empty_proof.deadend_transitions) > 0
    assert len(empty_proof.cycle) > 0

    load_loop = ext.parse_module_program("""(program
      (:entry "load-loop")
      (module "load-loop"
        (:arguments)
        (:registers
          (concept "x" 0)
        )
        (:entry "source")
        (:memory "source")
        (:features)
        (:transitions
          ("source" "source" (load (:conditions) (:concept (c_top)) (:register "x")))
        )
      )
    )""", planning_domain, repository)
    options = ext.GroundModuleProgramSearchOptions()
    load_proof = ext.prove_ground_solution(search_context, load_loop, options)
    assert load_proof.status == ext.ModuleProgramProofStatus.FAILURE
    assert len(load_proof.deadend_transitions) == 0
    assert len(load_proof.cycle) > 0


    with pytest.raises(RuntimeError, match=r'Unknown action "missing-action".*offset'):
        ext.parse_module_program("""(program
          (:entry "no-action")
          (module "no-action"
            (:arguments)
            (:registers)
            (:entry "source")
            (:memory "source" "target")
            (:features)
            (:transitions
              ("source" "target" (do (:conditions) (:action "missing-action") (:arguments)))
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

    program = ext.parse_module_program("""(program
      (:entry "empty")
      (module "empty"
        (:arguments)
        (:registers)
        (:entry "source")
        (:memory "source")
        (:features)
        (:transitions)
      )
    )""", planning_domain, repository)
    options = ext.LiftedModuleProgramSearchOptions()

    result = ext.prove_lifted_solution(search_context, program, options)

    assert result.status == ext.ModuleProgramProofStatus.FAILURE
    assert not result.is_successful()
