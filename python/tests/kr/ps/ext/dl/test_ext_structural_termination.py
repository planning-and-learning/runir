from pypddl_datasets import data_root

from pyrunir.kr.dl import ext as dl_ext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext import dl
from pyrunir.kr.ps.base.dl import NumericalChange
from pypddl.formalism import ParserOptions
from pytyr.formalism.planning import Parser

TERMINATING_MODULE = """(:module
    (:symbol term)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1)
    (:features
        (:boolean
            (:symbol unused)
            (:expression
                (b_nonempty
                    (c_top)
                )
            )
        )
        (:numerical
            (:symbol fn)
            (:expression
                (n_count
                    (c_atomic_state "ball")
                )
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto1)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:sketch
                    (:conditions
                        (greater_zero fn)
                    )
                    (:effects
                        (decreases fn)
                    )
                )
            )
        )
        (:rule
            (:symbol auto3)
            (:expression
                (:source-memory m1)
                (:target-memory m0)
                (:sketch
                    (:conditions)
                    (:effects
                        (unchanged fn)
                    )
                )
            )
        )
    )
)"""

NON_TERMINATING_MODULE = TERMINATING_MODULE.replace("(:symbol term)", "(:symbol nonterm)").replace(
    """(:effects
                        (decreases fn)
                    )""",
    """(:effects
                        (unchanged fn)
                    )""",
    1,
)


def _repositories():
    domain_path = data_root() / "classical" / "tests" / "gripper" / "domain.pddl"
    planning_domain = Parser(domain_path, ParserOptions()).get_domain()
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(planning_domain)
    repository = ext.RepositoryFactory().create(dl_repository)
    return planning_domain, repository


def test_ext_structural_termination_is_terminating():
    planning_domain, repository = _repositories()
    module = dl.parse_module(TERMINATING_MODULE, planning_domain, repository)

    result = dl.structural_termination(module)

    assert result.is_terminating()
    assert len(result.booleans) == 1
    assert result.counterexample is None


def test_ext_structural_termination_counterexample_spans_memory_states():
    planning_domain, repository = _repositories()
    module = dl.parse_module(NON_TERMINATING_MODULE, planning_domain, repository)

    result = dl.structural_termination(module)

    assert not result.is_terminating()
    counterexample = result.counterexample
    assert counterexample is not None

    (feature,) = result.numericals

    memory_states = {vertex.memory_state.get_name() for vertex in counterexample.vertices}
    assert memory_states == {"m0", "m1"}

    rules = {edge.rule.get_index() for edge in counterexample.edges}
    assert len(rules) == 2

    # Feature-change pairs over the numerical feature: both rules preserve it.
    changes = {dict(edge.numerical_changes)[feature] for edge in counterexample.edges}
    assert changes == {NumericalChange.UNCHANGED}


def test_ext_ceg_agrees_with_complete_sieve():
    planning_domain, repository = _repositories()
    module = dl.parse_module(NON_TERMINATING_MODULE, planning_domain, repository)

    complete = dl.structural_termination(module)
    ceg = dl.ceg_structural_termination(module)

    assert complete.is_terminating() == ceg.is_terminating() == False  # noqa: E712
    complete_rules = {edge.rule.get_index() for edge in complete.counterexample.edges}
    ceg_rules = {edge.rule.get_index() for edge in ceg.counterexample.edges}
    assert complete_rules == ceg_rules
