# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import gc
import sys
from collections.abc import Callable
from typing import TypeVar

import pytest
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
        repository = dl.semantics.ConstructorRepositoryFactory().create(gripper_planning_domain)
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
                semantics.ConceptData(_created(lambda: repository.insert(semantics.ConceptTopData())).get_index())
            )
        )
        atom = _created(lambda: repository.insert(semantics.QueryConceptData(concept.get_index(), columns)))
        num_tuples = sum(1 for _ in concept.evaluate(context))
    else:
        role = _created(
            lambda: repository.insert(
                semantics.RoleData(_created(lambda: repository.insert(semantics.RoleUniversalData())).get_index())
            )
        )
        atom = _created(lambda: repository.insert(semantics.QueryRoleData(role.get_index(), columns)))
        num_tuples = sum(1 for _ in role.evaluate(context))
    query = _created(lambda: repository.insert(semantics.QueryData(atom.get_index())))
    joined = _created(lambda: repository.insert(semantics.QueryJoinData(query.get_index(), query.get_index())))
    assert [column.get_name() for column in joined.get_columns()] == names
    query = _created(lambda: repository.insert(semantics.QueryData(joined.get_index())))
    if category == "Concept":
        concept_projected = _created(
            lambda: repository.insert(semantics.ConceptProjectData(query.get_index(), columns[::-1]))
        )
        assert [column.get_name() for column in concept_projected.get_columns()] == names[::-1]
        concept_projection = _created(lambda: repository.insert(semantics.ConceptData(concept_projected.get_index())))
        concept_projection_variant = concept_projection.get_variant()
        assert isinstance(concept_projection_variant, semantics.ConceptProject)
        assert concept_projection_variant == concept_projected
        projection_index: dl.semantics.ConceptIndex | dl.semantics.RoleIndex = concept_projection.get_index()
    else:
        role_projected = _created(
            lambda: repository.insert(semantics.RoleProjectData(query.get_index(), columns[::-1]))
        )
        assert [column.get_name() for column in role_projected.get_columns()] == names[::-1]
        role_projection = _created(lambda: repository.insert(semantics.RoleData(role_projected.get_index())))
        role_projection_variant = role_projection.get_variant()
        assert isinstance(role_projection_variant, semantics.RoleProject)
        assert role_projection_variant == role_projected
        projection_index = role_projection.get_index()

    count = _created(lambda: repository.insert(semantics.NumericalCountData(projection_index)))
    numerical = _created(lambda: repository.insert(semantics.NumericalData(count.get_index())))
    nonempty = _created(lambda: repository.insert(semantics.BooleanNonemptyData(query.get_index())))
    boolean = _created(lambda: repository.insert(semantics.BooleanData(nonempty.get_index())))
    numerical_variant = numerical.get_variant()
    assert isinstance(numerical_variant, semantics.NumericalCount)
    assert numerical_variant == count
    boolean_variant = boolean.get_variant()
    assert isinstance(boolean_variant, semantics.BooleanNonempty)
    assert boolean_variant == nonempty
    assert numerical.evaluate(context).get() == num_tuples
    assert boolean.evaluate(context).get() is True


@pytest.mark.parametrize(("category", "names"), [("Concept", ["object"]), ("Role", ["left", "right"])])
def test_repository_constructs_queries_and_semantic_wrappers(
    gripper_planning_domain: PlanningDomain,
    ground_gripper_search_context: GroundTaskSearchContext,
    category: str,
    names: list[str],
) -> None:
    semantics = dl.semantics
    repository = semantics.ConstructorRepositoryFactory().create(gripper_planning_domain)
    assert dl.ConstructorRepositoryFactory is semantics.ConstructorRepositoryFactory
    assert dl.ConstructorRepository is semantics.ConstructorRepository

    columns: list[dl.semantics.QueryColumnIndex] = []
    for name in names:
        data = dl.semantics.QueryColumnData()
        data.name = name
        columns.append(repository.insert(data)[0].get_index())

    search = ground_gripper_search_context
    task = GroundTaskContext(DomainContext(gripper_planning_domain), search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    storage = semantics.EvaluationStorage(task.dl_denotation_repository)
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, storage)
    _check_queries(repository, context, category, columns, names)


def _check_concept_argument(repository: dl.ConstructorRepository) -> None:
    data = dl.ConceptArgumentData("reference", dl.ConceptArgumentIdentifier(3))
    reference = _created(lambda: repository.insert(data))
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier


def _check_role_argument(repository: dl.ConstructorRepository) -> None:
    data = dl.RoleArgumentData("reference", dl.RoleArgumentIdentifier(3))
    reference = _created(lambda: repository.insert(data))
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier


def _check_boolean_argument(repository: dl.ConstructorRepository) -> None:
    data = dl.BooleanArgumentData("reference", dl.BooleanArgumentIdentifier(3))
    reference = _created(lambda: repository.insert(data))
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier


def _check_numerical_argument(repository: dl.ConstructorRepository) -> None:
    data = dl.NumericalArgumentData("reference", dl.NumericalArgumentIdentifier(3))
    reference = _created(lambda: repository.insert(data))
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier


def _check_concept_register(repository: dl.ConstructorRepository) -> None:
    data = dl.ConceptRegisterData("reference", dl.ConceptRegisterIdentifier(3))
    reference = _created(lambda: repository.insert(data))
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier


def _check_role_register(repository: dl.ConstructorRepository) -> None:
    data = dl.RoleRegisterData("reference", dl.RoleRegisterIdentifier(3))
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
    assert dl.ConstructorRepositoryFactory is dl.semantics.ConstructorRepositoryFactory
    assert dl.ConstructorRepository is dl.semantics.ConstructorRepository
    check(dl.semantics.ConstructorRepositoryFactory().create(gripper_planning_domain))
