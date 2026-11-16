# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

from fixture_utils import record
from pytyr.formalism.planning import PlanningDomain

from pyrunir.kr import dl


def test_value_and_view_constructors_build_equal_data(gripper_planning_domain: PlanningDomain) -> None:
    semantics = dl.semantics
    repository = semantics.ConstructorRepository(gripper_planning_domain)

    top = repository.insert(semantics.ConceptSemanticTopData())[0]
    concept = repository.insert(record(semantics.ConstructorConceptData, variant=top.index()))[0]
    bot = repository.insert(semantics.ConceptSemanticBotData())[0]
    other = repository.insert(record(semantics.ConstructorConceptData, variant=bot.index()))[0]

    by_index = semantics.ConceptSemanticIntersectionData(concept.index(), other.index())
    by_view = record(semantics.ConceptSemanticIntersectionData, left_operand=concept, right_operand=other)
    assert by_index == by_view
    assert repository.insert(by_index)[0] == repository.insert(by_view)[0]

    column = repository.insert(record(semantics.QueryColumnData, name="x"))[0]
    assert column.get_name() == "x"
    query_by_index = record(semantics.QueryConceptData, operand=concept.index(), columns=[column.index()])
    query_by_view = record(semantics.QueryConceptData, operand=concept, columns=[column])
    assert query_by_index == query_by_view


def test_view_variant_constructor_builds_equal_data(gripper_planning_domain: PlanningDomain) -> None:
    semantics = dl.semantics
    repository = semantics.ConstructorRepository(gripper_planning_domain)

    top = repository.insert(semantics.ConceptSemanticTopData())[0]
    assert record(semantics.ConstructorConceptData, variant=top) == record(
        semantics.ConstructorConceptData, variant=top.index()
    )
