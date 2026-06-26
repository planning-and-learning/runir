from pathlib import Path

from pyrunir.kr.dl.base.semantics import ConstructorRepositoryFactory
from pyrunir.kr.ps.base import RepositoryFactory
from pyrunir.kr.ps.base.dl import (
    BooleanFeature,
    NumericalChange,
    NumericalFeature,
    incomplete_structural_termination,
    parse_sketch,
    structural_termination,
)
from pypddl.formalism import ParserOptions
from pytyr.formalism.planning import Parser

OSCILLATOR = """(:sketch
    (:features
        (:boolean
            (:symbol b1)
            (:expression
                (b_nonempty
                    (c_atomic_state
                        "at-robby"))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto1)
            (:expression
                (:conditions
                    (negative b1)
                )
                (:effects
                    (positive b1)
                )
            )
        )
        (:rule
            (:symbol auto2)
            (:expression
                (:conditions
                    (positive b1)
                )
                (:effects
                    (negative b1)
                )
            )
        )
    )
)
"""

TPP = """(:sketch
    (:features
        (:numerical
            (:symbol fb)
            (:expression
                (n_count
                    (c_atomic_state
                        "ball"))
            )
        )
        (:numerical
            (:symbol fl)
            (:expression
                (n_count
                    (c_atomic_state
                        "room"))
            )
        )
        (:numerical
            (:symbol fn)
            (:expression
                (n_count
                    (c_atomic_state
                        "gripper"))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto3)
            (:expression
                (:conditions
                    (greater_zero fb)
                )
                (:effects
                    (decreases fb)
                    (unchanged fl)
                    (unchanged fn)
                )
            )
        )
        (:rule
            (:symbol auto4)
            (:expression
                (:conditions
                    (greater_zero fl)
                )
                (:effects
                    (unchanged fn)
                    (decreases fl)
                )
            )
        )
        (:rule
            (:symbol auto5)
            (:expression
                (:conditions
                    (greater_zero fn)
                )
                (:effects
                    (decreases fn)
                )
            )
        )
    )
)
"""


def make_repository():
    root = Path(__file__).resolve().parents[6]
    domain_path = root / "data" / "planning-benchmarks" / "tests" / "classical" / "gripper" / "domain.pddl"
    planning_domain = Parser(domain_path, ParserOptions()).get_domain()
    dl_repository = ConstructorRepositoryFactory().create(planning_domain)
    repository = RepositoryFactory().create(dl_repository)
    return planning_domain, repository


def test_structural_termination_tpp_sketch_is_terminating():
    domain, repository = make_repository()
    sketch = parse_sketch(TPP, domain, repository)

    result = structural_termination(sketch)

    assert result.is_terminating()
    assert result.get_counterexample() is None
    assert len(result.get_numericals()) == 3
    assert all(isinstance(feature, NumericalFeature) for feature in result.get_numericals())


def test_structural_termination_oscillator_counterexample_has_dict_valuations():
    domain, repository = make_repository()
    sketch = parse_sketch(OSCILLATOR, domain, repository)

    result = structural_termination(sketch)

    assert not result.is_terminating()
    counterexample = result.get_counterexample()
    assert counterexample is not None
    assert counterexample.get_num_vertices() == 2
    assert counterexample.get_num_edges() == 2

    (feature,) = result.get_booleans()
    assert isinstance(feature, BooleanFeature)

    # Dict-shaped valuations: one vertex per truth value of b1.
    valuations = {vertex.get_booleans()[feature] for vertex in counterexample.get_vertices()}
    assert valuations == {True, False}

    # The two-cycle uses both rules; each edge flips b1.
    rules = {edge.get_rule().get_index() for edge in counterexample.get_edges()}
    assert rules == {rule.get_index() for rule in sketch.get_rules()}
    for edge in counterexample.get_edges():
        vertices = counterexample.get_vertices()
        source = vertices[edge.get_source()].get_booleans()[feature]
        target = vertices[edge.get_target()].get_booleans()[feature]
        assert source != target


def test_structural_termination_edge_changes_are_dict_shaped():
    domain, repository = make_repository()
    sketch = parse_sketch(
        """(:sketch
    (:features
        (:numerical
            (:symbol n1)
            (:expression
                (n_count
                    (c_atomic_state
                        "ball"))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto6)
            (:expression
                (:conditions
                    (greater_zero n1)
                )
                (:effects
                    (decreases n1)
                )
            )
        )
        (:rule
            (:symbol auto7)
            (:expression
                (:conditions)
                (:effects
                    (increases n1)
                )
            )
        )
    )
)
""",
        domain,
        repository,
    )

    result = structural_termination(sketch)

    assert not result.is_terminating()
    (feature,) = result.get_numericals()
    changes = {edge.get_numerical_changes()[feature] for edge in result.get_counterexample().get_edges()}
    assert changes == {NumericalChange.DECREASES, NumericalChange.INCREASES}


def test_incomplete_structural_termination_reports_blocking_reasons():
    domain, repository = make_repository()
    sketch = parse_sketch(OSCILLATOR, domain, repository)

    result = incomplete_structural_termination(sketch)

    assert not result.is_terminating()
    assert len(result.surviving_rules) == 2
    for surviving in result.surviving_rules:
        (reason,) = surviving.blocking_reasons
        assert isinstance(reason.feature, BooleanFeature)
        (opposing,) = reason.opposing_rules
        assert opposing.get_index() != surviving.rule.get_index()


def test_incomplete_structural_termination_tpp_is_terminating():
    domain, repository = make_repository()
    sketch = parse_sketch(TPP, domain, repository)

    result = incomplete_structural_termination(sketch)

    assert result.is_terminating()
    assert result.surviving_rules == []
