from pathlib import Path

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
from pytyr.common import ExecutionContext
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

    formatted_on = str(modules[0])
    reparsed_on = ext.parse_module(formatted_on, planning_domain, repository)

    assert reparsed_on.get_name() == "on"
    assert str(reparsed_on) == formatted_on


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

    descriptions = ext.ModuleFactory.create_bonet_et_al_icaps2024_descriptions()
    descriptions.append("""(module "entry"
      (:arguments)
      (:registers)
      (:entry "m0")
      (:memory "m0" "m1")
      (:features)
      (:transitions
        ("m0" "m1" (call (:conditions) (:callee "blocks") (:arguments (r_atomic_goal "on" true))))
      )
    )""")
    modules = ext.parse_modules(descriptions, planning_domain, repository)

    options = ext.GroundModuleExecutionOptions()
    options.max_arity = 1
    options.max_steps = 1024
    options.max_load_steps = 1024

    result = ext.execute_ground_solution(search_context, modules[-1], modules, options)

    assert result.status == ext.ModuleExecutionStatus.SUCCESS
    assert result.is_successful()
    assert result.num_steps > 0


def test_executor_reports_structured_failure_statuses_from_python():
    search_context, planning_domain, ground_task = _ground_context_and_domain()
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(ground_task)
    repository = ext.RepositoryFactory().create(dl_repository)

    empty_module = ext.parse_module("""(module "empty"
      (:arguments)
      (:registers)
      (:entry "source")
      (:memory "source")
      (:features)
      (:transitions)
    )""", planning_domain, repository)
    options = ext.GroundModuleExecutionOptions()
    options.max_steps = 0
    assert ext.execute_ground_solution(search_context, empty_module, options).status == ext.ModuleExecutionStatus.STEP_LIMIT_REACHED

    load_loop = ext.parse_module("""(module "load-loop"
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
    )""", planning_domain, repository)
    options = ext.GroundModuleExecutionOptions()
    options.max_load_steps = 1
    assert ext.execute_ground_solution(search_context, load_loop, options).status == ext.ModuleExecutionStatus.LOAD_LIMIT_REACHED

    caller = """(module "caller"
      (:arguments)
      (:registers)
      (:entry "source")
      (:memory "source" "target")
      (:features)
      (:transitions
        ("source" "target" (call (:conditions) (:callee "callee") (:arguments)))
      )
    )"""
    callee = """(module "callee"
      (:arguments
        (concept "x" 0)
      )
      (:registers)
      (:entry "target")
      (:memory "target")
      (:features)
      (:transitions)
    )"""
    modules = ext.parse_modules([caller, callee], planning_domain, repository)
    assert ext.execute_ground_solution(search_context, modules[0], modules).status == ext.ModuleExecutionStatus.MALFORMED_CALL

    no_action = ext.parse_module("""(module "no-action"
      (:arguments)
      (:registers)
      (:entry "source")
      (:memory "source" "target")
      (:features)
      (:transitions
        ("source" "target" (do (:conditions) (:action "missing-action") (:arguments)))
      )
    )""", planning_domain, repository)
    assert ext.execute_ground_solution(search_context, no_action).status == ext.ModuleExecutionStatus.NO_APPLICABLE_ACTION


def test_lifted_executor_binding_reports_step_limit():
    planning_task, planning_domain = _planning_task_and_domain()
    execution_context = ExecutionContext(1)
    lifted_task = Task(planning_task)
    search_context = LiftedTaskSearchContext(lifted_task, execution_context)
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(lifted_task)
    repository = ext.RepositoryFactory().create(dl_repository)

    module = ext.parse_module("""(module "empty"
      (:arguments)
      (:registers)
      (:entry "source")
      (:memory "source")
      (:features)
      (:transitions)
    )""", planning_domain, repository)
    options = ext.LiftedModuleExecutionOptions()
    options.max_steps = 0

    result = ext.execute_lifted_solution(search_context, module, options)

    assert result.status == ext.ModuleExecutionStatus.STEP_LIMIT_REACHED
    assert not result.is_successful()
