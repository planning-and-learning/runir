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

SEPARATED_BOOLEAN_RULES_MODULE = """(:module
    (:symbol separated)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1)
    (:features
        (:boolean
            (:symbol b)
            (:expression (b_nonempty (c_atomic_state "ball")))
        )
    )
    (:rules
        (:rule
            (:symbol r1)
            (:expression
                (:source-memory m0)
                (:target-memory m0)
                (:sketch
                    (:conditions (negative b))
                    (:effects (positive b))
                )
            )
        )
        (:rule
            (:symbol r2)
            (:expression
                (:source-memory m1)
                (:target-memory m1)
                (:sketch
                    (:conditions (positive b))
                    (:effects (negative b))
                )
            )
        )
    )
)"""

PROJECTED_COMPONENTS_MODULE = """(:module
    (:symbol projected)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0 m1 m2)
    (:features
        (:boolean
            (:symbol b)
            (:expression (b_nonempty (c_atomic_state "ball")))
        )
        (:numerical
            (:symbol n0)
            (:expression (n_count (c_atomic_state "ball")))
        )
        (:numerical
            (:symbol n1)
            (:expression (n_count (c_atomic_state "room")))
        )
    )
    (:rules
        (:rule
            (:symbol to_true)
            (:expression
                (:source-memory m0)
                (:target-memory m0)
                (:sketch
                    (:conditions (negative b))
                    (:effects (positive b))
                )
            )
        )
        (:rule
            (:symbol to_false)
            (:expression
                (:source-memory m0)
                (:target-memory m0)
                (:sketch
                    (:conditions (positive b))
                    (:effects (negative b))
                )
            )
        )
        (:rule
            (:symbol keep_n1)
            (:expression
                (:source-memory m1)
                (:target-memory m1)
                (:sketch
                    (:conditions)
                    (:effects (unchanged n1))
                )
            )
        )
        (:rule
            (:symbol keep_n0)
            (:expression
                (:source-memory m2)
                (:target-memory m2)
                (:sketch
                    (:conditions)
                    (:effects (unchanged n0))
                )
            )
        )
    )
)"""


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
    assert dl.structural_termination(module, max_features=1, use_incomplete_preprocessing=False).is_terminating()


def test_ext_structural_termination_counterexample_spans_memory_states():
    planning_domain, repository = _repositories()
    module = dl.parse_module(NON_TERMINATING_MODULE, planning_domain, repository)

    result = dl.structural_termination(module)

    assert not result.is_terminating()
    counterexample = result.counterexample
    assert counterexample is not None

    (feature,) = result.numericals

    vertices = [counterexample.get_vertex_property(vertex) for vertex in counterexample.get_vertex_indices()]
    memory_states = {vertex.memory_state.get_name() for vertex in vertices}
    assert memory_states == {"m0", "m1"}
    for vertex in vertices:
        assert len(vertex.boolean_values) == len(result.booleans)
        assert len(vertex.numerical_values) == len(result.numericals)

    edges = [counterexample.get_edge_property(edge) for edge in counterexample.get_edge_indices()]
    rules = {edge.rule.get_index() for edge in edges}
    assert len(rules) == 2

    # Positional native labels align with the result's feature indices.
    changes = {dict(zip(result.numericals, edge.numerical_changes))[feature] for edge in edges}
    assert changes == {NumericalChange.UNCHANGED}


def test_ext_structural_termination_lifts_projected_components():
    planning_domain, repository = _repositories()
    module = dl.parse_module(PROJECTED_COMPONENTS_MODULE, planning_domain, repository)

    result = dl.structural_termination(module)

    assert not result.is_terminating()
    assert result.counterexample is not None
    assert len(result.booleans) == 1
    assert len(result.numericals) == 2

    vertices = [result.counterexample.get_vertex_property(vertex) for vertex in result.counterexample.get_vertex_indices()]
    assert {vertex.memory_state.get_name() for vertex in vertices} == {"m0", "m1", "m2"}
    positive_n0 = False
    positive_n1 = False
    for vertex in vertices:
        assert len(vertex.boolean_values) == len(result.booleans)
        assert len(vertex.numerical_values) == len(result.numericals)
        if vertex.memory_state.get_name() == "m0":
            assert not any(vertex.numerical_values)
        elif vertex.memory_state.get_name() == "m1":
            assert not any(vertex.boolean_values)
            assert not vertex.numerical_values[0]
            positive_n1 |= vertex.numerical_values[1]
        else:
            assert not any(vertex.boolean_values)
            assert not vertex.numerical_values[1]
            positive_n0 |= vertex.numerical_values[0]
    assert positive_n0
    assert positive_n1

    edges = [result.counterexample.get_edge_property(edge) for edge in result.counterexample.get_edge_indices()]
    assert {edge.rule.get_symbol() for edge in edges} == {"keep_n0", "keep_n1", "to_false", "to_true"}
    assert all(len(edge.numerical_changes) == len(result.numericals) for edge in edges)
    expected_changes = {
        "keep_n0": [NumericalChange.UNCHANGED, NumericalChange.UNCONSTRAINED],
        "keep_n1": [NumericalChange.UNCONSTRAINED, NumericalChange.UNCHANGED],
        "to_false": [NumericalChange.UNCONSTRAINED, NumericalChange.UNCONSTRAINED],
        "to_true": [NumericalChange.UNCONSTRAINED, NumericalChange.UNCONSTRAINED],
    }
    for edge in edges:
        assert edge.numerical_changes == expected_changes[edge.rule.get_symbol()]


def test_ext_incomplete_structural_termination_uses_memory_components():
    planning_domain, repository = _repositories()
    module = dl.parse_module(SEPARATED_BOOLEAN_RULES_MODULE, planning_domain, repository)

    result = dl.incomplete_structural_termination(module)

    assert result.is_terminating()
    assert result.status == dl.IncompleteStructuralTerminationStatus.TERMINATING
    assert len(result.booleans) == 1
    assert result.numericals == []
    assert result.surviving_rules == []


def test_ext_incomplete_structural_termination_accepts_module_program():
    planning_domain, repository = _repositories()
    program = dl.parse_module_program(f"(:program (:entry nonterm) {NON_TERMINATING_MODULE})", planning_domain, repository)

    result = dl.incomplete_structural_termination(program)
    complete_result = dl.structural_termination(program, max_features=1, use_incomplete_preprocessing=False)

    assert not complete_result.is_terminating()
    assert not result.is_terminating()
    assert len(result.module_results) == 1
    assert result.recursive_call_rules == []
