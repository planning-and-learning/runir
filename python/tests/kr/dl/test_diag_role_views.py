# Diagnostic only (macOS: role views raise "Symbol index not found"); not to be merged.

from fixture_utils import record
from pytyr.formalism.planning import PlanningDomain

from pyrunir.datasets import GroundTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext, dl


def _try(label, f):
    try:
        print(f"DIAG {label}: {f()!r}", flush=True)
    except Exception as e:  # noqa: BLE001
        print(f"DIAG {label}: RAISES {type(e).__name__}: {e}", flush=True)


def test_diag_role_views(
    gripper_planning_domain: PlanningDomain, ground_gripper_search_context: GroundTaskSearchContext
) -> None:
    s = dl.semantics
    repository = s.ConstructorRepository(gripper_planning_domain)
    search = ground_gripper_search_context
    task = GroundTaskContext(DomainContext(gripper_planning_domain), search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    context = s.GroundStateEvaluationContext(state, task.dl_builder, s.EvaluationStorage(task.dl_denotation_repository))

    top = repository.insert(
        record(s.ConstructorConceptData, variant=repository.insert(s.ConceptSemanticTopData())[0].index())
    )[0]
    universal = repository.insert(
        record(s.ConstructorRoleData, variant=repository.insert(s.RoleSemanticUniversalData())[0].index())
    )[0]
    c = s.evaluate(top, context)
    r = s.evaluate(universal, context)
    for name, d in (("concept", c), ("role", r)):
        _try(f"{name} type", lambda d=d: type(d))
        _try(f"{name} dir", lambda d=d: [a for a in dir(d) if not a.startswith("_")])
        _try(f"{name} num_objects", lambda d=d: d.get_num_objects())
        _try(f"{name} blocks", lambda d=d: [int(b) for b in d.get_blocks()])
        _try(f"{name} storage_bits", lambda d=d: d.storage_bits())
        _try(f"{name} indices", lambda d=d: list(d.indices()))
        _try(f"{name} views type", lambda d=d: type(d.views()))
        _try(f"{name} views len", lambda d=d: len(d.views()))
        _try(f"{name} views first", lambda d=d: next(iter(d.views())))
        _try(f"{name} views list", lambda d=d: list(d.views()))
    _try("role indices(source 0)", lambda: list(r.indices(c.indices()[0] if hasattr(c.indices(), "__getitem__") else next(iter(c.indices())))))
    _try("role views_from(first object)", lambda: [str(o.get_name()) for o in r.views_from(next(iter(c.views())))])
    _try("concept views names", lambda: [str(o.get_name()) for o in c.views()])
    assert False, "diagnostic"
