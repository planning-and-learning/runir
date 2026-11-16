# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import gc
import sys
from collections.abc import Callable
from typing import TypeVar

import pytest
from fixture_utils import record
from pytyr.formalism.planning import PlanningDomain

from pyrunir.datasets import GroundTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext, dl

T = TypeVar("T")


def _created(insert: Callable[[], tuple[T, bool]]) -> T:
    """Insert once, check that reinserting the same data returns the same view, and return it."""
    result = insert()[0]
    assert insert()[0] == result
    return result


def test_insert_returns_creation_flag_and_retains_owner_on_view(gripper_planning_domain: PlanningDomain) -> None:
    def create_view() -> dl.semantics.QueryColumn:
        repository = dl.semantics.ConstructorRepository(gripper_planning_domain)
        data = dl.semantics.QueryColumnData()
        data.name = "retained"
        before = sys.getrefcount(repository)
        result = repository.insert(data)
        assert isinstance(result, tuple)
        view, inserted = result
        assert inserted is True
        assert repository.insert(data) == (view, False)
        assert not hasattr(repository, "get_or_create")
        del result
        assert sys.getrefcount(repository) > before
        return view

    view = create_view()
    gc.collect()
    assert view.get_name() == "retained"


def _check_queries(
    repository: dl.semantics.ConstructorRepository,
    context: dl.semantics.GroundStateEvaluationContext,
    category: str,
    columns: list[dl.semantics.QueryColumnIndex],
    names: list[str],
) -> None:
    """Build concept or role queries over `columns`, join and project them, and evaluate counts over them."""
    semantics = dl.semantics
    if category == "Concept":
        concept = _created(
            lambda: repository.insert(
                record(
                    semantics.ConstructorConceptData,
                    variant=_created(lambda: repository.insert(semantics.ConceptSemanticTopData())).index(),
                )
            )
        )
        atom = _created(
            lambda: repository.insert(record(semantics.QueryConceptData, operand=concept.index(), columns=columns))
        )
        num_tuples = len(semantics.evaluate(concept, context).views())
    else:
        role = _created(
            lambda: repository.insert(
                record(
                    semantics.ConstructorRoleData,
                    variant=_created(lambda: repository.insert(semantics.RoleSemanticUniversalData())).index(),
                )
            )
        )
        atom = _created(
            lambda: repository.insert(record(semantics.QueryRoleData, operand=role.index(), columns=columns))
        )
        num_tuples = len(semantics.evaluate(role, context).views())
    query = _created(lambda: repository.insert(record(semantics.QueryData, variant=atom.index())))
    joined = _created(
        lambda: repository.insert(
            record(semantics.QueryJoinData, left_operand=query.index(), right_operand=query.index())
        )
    )
    assert [column.get_name() for column in joined.get_columns()] == names
    query = _created(lambda: repository.insert(record(semantics.QueryData, variant=joined.index())))
    if category == "Concept":
        concept_projected = _created(
            lambda: repository.insert(
                record(semantics.QueryProjectionConceptData, operand=query.index(), columns=columns[::-1])
            )
        )
        assert [column.get_name() for column in concept_projected.get_columns()] == names[::-1]
        concept_projection = _created(
            lambda: repository.insert(record(semantics.ConstructorConceptData, variant=concept_projected.index()))
        )
        concept_projection_variant = concept_projection.get_variant()
        assert isinstance(concept_projection_variant, semantics.QueryProjectionConcept)
        assert concept_projection_variant == concept_projected
        projection_index: dl.semantics.ConstructorConceptIndex | dl.semantics.ConstructorRoleIndex = (
            concept_projection.index()
        )
    else:
        role_projected = _created(
            lambda: repository.insert(
                record(semantics.QueryProjectionRoleData, operand=query.index(), columns=columns[::-1])
            )
        )
        assert [column.get_name() for column in role_projected.get_columns()] == names[::-1]
        role_projection = _created(
            lambda: repository.insert(record(semantics.ConstructorRoleData, variant=role_projected.index()))
        )
        role_projection_variant = role_projection.get_variant()
        assert isinstance(role_projection_variant, semantics.QueryProjectionRole)
        assert role_projection_variant == role_projected
        projection_index = role_projection.index()

    count = _created(lambda: repository.insert(record(semantics.NumericalSemanticCountData, operand=projection_index)))
    numerical = _created(lambda: repository.insert(record(semantics.ConstructorNumericalData, variant=count.index())))
    nonempty = _created(lambda: repository.insert(record(semantics.BooleanSemanticNonemptyData, operand=query.index())))
    boolean = _created(lambda: repository.insert(record(semantics.ConstructorBooleanData, variant=nonempty.index())))
    numerical_variant = numerical.get_variant()
    assert isinstance(numerical_variant, semantics.NumericalSemanticCount)
    assert numerical_variant == count
    boolean_variant = boolean.get_variant()
    assert isinstance(boolean_variant, semantics.BooleanSemanticNonempty)
    assert boolean_variant == nonempty
    assert semantics.evaluate(numerical, context).get_value() == num_tuples
    assert semantics.evaluate(boolean, context).get_value() is True


@pytest.mark.parametrize(("category", "names"), [("Concept", ["object"]), ("Role", ["left", "right"])])
def test_repository_constructs_queries_and_semantic_wrappers(
    gripper_planning_domain: PlanningDomain,
    ground_gripper_search_context: GroundTaskSearchContext,
    category: str,
    names: list[str],
) -> None:
    semantics = dl.semantics
    repository = semantics.ConstructorRepository(gripper_planning_domain)
    assert dl.ConstructorRepository is semantics.ConstructorRepository
    assert dl.ConstructorRepository is semantics.ConstructorRepository

    columns: list[dl.semantics.QueryColumnIndex] = []
    for name in names:
        data = dl.semantics.QueryColumnData()
        data.name = name
        columns.append(repository.insert(data)[0].index())

    search = ground_gripper_search_context
    task = GroundTaskContext(DomainContext(gripper_planning_domain), search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    storage = semantics.EvaluationStorage(task.dl_denotation_repository)
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, storage)
    _check_queries(repository, context, category, columns, names)


def _check_concept_argument(repository: dl.ConstructorRepository) -> None:
    data = record(dl.ArgumentConceptData, name="reference", identifier=dl.ConceptArgumentIdentifier(3))
    reference = _created(lambda: repository.insert(data))
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier


def _check_role_argument(repository: dl.ConstructorRepository) -> None:
    data = record(dl.ArgumentRoleData, name="reference", identifier=dl.RoleArgumentIdentifier(3))
    reference = _created(lambda: repository.insert(data))
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier


def _check_boolean_argument(repository: dl.ConstructorRepository) -> None:
    data = record(dl.ArgumentBooleanData, name="reference", identifier=dl.BooleanArgumentIdentifier(3))
    reference = _created(lambda: repository.insert(data))
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier


def _check_numerical_argument(repository: dl.ConstructorRepository) -> None:
    data = record(dl.ArgumentNumericalData, name="reference", identifier=dl.NumericalArgumentIdentifier(3))
    reference = _created(lambda: repository.insert(data))
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier


def _check_concept_register(repository: dl.ConstructorRepository) -> None:
    data = record(dl.RegisterConceptData, name="reference", identifier=dl.ConceptRegisterIdentifier(3))
    reference = _created(lambda: repository.insert(data))
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier


def _check_role_register(repository: dl.ConstructorRepository) -> None:
    data = record(dl.RegisterRoleData, name="reference", identifier=dl.RoleRegisterIdentifier(3))
    reference = _created(lambda: repository.insert(data))
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier


@pytest.mark.parametrize(
    "check",
    [
        _check_concept_argument,
        _check_role_argument,
        _check_boolean_argument,
        _check_numerical_argument,
        _check_concept_register,
        _check_role_register,
    ],
    ids=["ConceptArgument", "RoleArgument", "BooleanArgument", "NumericalArgument", "ConceptRegister", "RoleRegister"],
)
def test_repository_constructs_references(
    gripper_planning_domain: PlanningDomain, check: Callable[[dl.ConstructorRepository], None]
) -> None:
    assert dl.ConstructorRepository is dl.semantics.ConstructorRepository
    assert dl.ConstructorRepository is dl.semantics.ConstructorRepository
    check(dl.semantics.ConstructorRepository(gripper_planning_domain))
