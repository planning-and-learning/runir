# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import gc

import pytest
from pytyr.formalism.planning import Object, ObjectIndex, PlanningDomain
from pyyggdrasil import database
from pyyggdrasil.serialization import Dictionaries

from pyrunir.datasets import GroundTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext, UndefinedSymbolError
from pyrunir.kr.dl import semantics
from pyrunir.kr.ps.base import Sketch
from pyrunir.kr.ps.base.dl import parse_sketch
from pyrunir.kr.ps.ext.dl import parse_program
from pyrunir.kr.uns.dl import parse_classifier
from pyrunir.serialization import register_table, serialize, table


def _row_values(rows: database.RelationView) -> list[int]:
    values: list[int] = []
    for row in rows:
        for value in row:
            assert isinstance(value, int)
            values.append(value)
    return values


def _boolean_arg(expression: semantics.Boolean) -> semantics.Concept | semantics.Role | semantics.Query:
    variant = expression.get_variant()
    assert isinstance(variant, semantics.BooleanNonempty)
    return variant.get_arg()


def _numerical_arg(expression: semantics.Numerical) -> semantics.Concept | semantics.Role | semantics.Query:
    variant = expression.get_variant()
    assert isinstance(variant, semantics.NumericalCount)
    return variant.get_arg()


def _concept_query(expression: semantics.Concept) -> semantics.Query:
    variant = expression.get_variant()
    assert isinstance(variant, semantics.ConceptProject)
    return variant.get_arg()


def _sketch(expression: str, category: str, domain: PlanningDomain, context: DomainContext) -> Sketch:
    return parse_sketch(
        f"(:sketch (:features (:{category} (:symbol value) (:expression {expression}))) (:rules))",
        domain,
        context.base_repository,
    )


@pytest.mark.parametrize("family", ["base", "ext", "uns"])
def test_nested_query_owner_round_trip_bindings_complexity_and_serialization(
    gripper_planning_domain: PlanningDomain, ground_gripper_search_context: GroundTaskSearchContext, family: str
) -> None:
    context = DomainContext(gripper_planning_domain)
    source = '(b_nonempty (q_project (ball) (q_atomic_state "at" (ball room))))'
    feature = f"(:boolean (:symbol value) (:expression {source}))"
    if family == "base":
        owner = parse_sketch(
            "".join(
                (
                    f"(:sketch (:features {feature}) (:rules ",
                    "(:rule (:symbol keep) (:expression ",
                    "(:conditions (positive value)) (:effects (unchanged value))))))",
                )
            ),
            gripper_planning_domain,
            context.base_repository,
        )
        expression = owner.get_boolean_features()[0].get_expression()
    elif family == "ext":
        owner = parse_program(
            "".join(
                (
                    "(:program (:entry root) (:module (:symbol root) (:arguments) (:registers) ",
                    f"(:entry m0) (:memory m0) (:features {feature}) (:rules)))",
                )
            ),
            gripper_planning_domain,
            context.ext_repository,
        )
        expression = owner.get_entry_module().get_boolean_features()[0].get_expression()
    else:
        owner = parse_classifier(
            f"(:classifier (:symbol root) (:features {feature}) (:expression (or (and value))))",
            gripper_planning_domain,
            context.uns_repository,
        )
        expression = owner.get_features()[0].get_expression()

    task = GroundTaskContext(context, ground_gripper_search_context)
    state = ground_gripper_search_context.state_repository.get_initial_state(
        ground_gripper_search_context.axiom_evaluator
    )
    storage = semantics.EvaluationStorage(task.dl_denotation_repository)
    bindings = (
        (
            task.dl_denotation_repository.insert(semantics.CallArgumentsData())[0],
            task.dl_denotation_repository.insert(semantics.RegisterValuesData())[0],
        )
        if family == "ext"
        else ()
    )
    evaluation_context = semantics.GroundStateEvaluationContext(state, task.dl_builder, storage, *bindings)
    assert expression.evaluate(evaluation_context).get() is True
    persistent_caches = semantics.DenotationCaches()
    repository = task.dl_denotation_repository
    retained_context = semantics.GroundStateEvaluationContext(
        state, task.dl_builder, persistent_caches, repository, storage, *bindings
    )
    retained = expression.evaluate(retained_context)
    query = _boolean_arg(expression)
    assert isinstance(query, semantics.Query)
    retained_query = query.evaluate(retained_context)
    relations = task.dl_denotation_repository.get_relation_repository()
    assert isinstance(relations, database.RelationRepository)
    assert isinstance(retained_query, database.RelationView)
    assert isinstance(retained_query.get_index(), database.RelationIndex)
    assert all(isinstance(row, database.RelationRow) for row in retained_query)
    assert all(
        isinstance(repository.get_object(ObjectIndex(object_)), Object) for object_ in _row_values(retained_query)
    )
    assert relations.rename(retained_query, list(retained_query.columns())) == retained_query
    retained_rows = {tuple(row) for row in retained_query}
    storage.reset_dynamic()
    assert expression.evaluate(evaluation_context).get() is True
    storage.reset_all()
    assert retained.get() is True
    assert {tuple(row) for row in retained_query} == retained_rows
    assert expression.evaluate(evaluation_context).get() is True
    ordinary_query = query.evaluate(evaluation_context)
    with pytest.raises(ValueError, match="source in this repository"):
        relations.rename(ordinary_query, list(ordinary_query.columns()))
    del evaluation_context, retained_context, persistent_caches, storage, repository, relations, task, state, bindings
    gc.collect()
    assert retained.get() is True
    assert {tuple(row) for row in retained_query} == retained_rows
    assert {tuple(row) for row in ordinary_query} == retained_rows

    formatted = str(owner)
    assert source in formatted
    fresh_context = DomainContext(gripper_planning_domain)
    if family == "base":
        reparsed = parse_sketch(formatted, gripper_planning_domain, fresh_context.base_repository)
        assert len(reparsed.get_rules()) == 1
    elif family == "ext":
        reparsed = parse_program(formatted, gripper_planning_domain, fresh_context.ext_repository)
    else:
        reparsed = parse_classifier(formatted, gripper_planning_domain, fresh_context.uns_repository)
    assert str(reparsed) == formatted

    assert str(expression) == source
    assert expression.syntactic_complexity() == 3
    assert [column.get_name() for column in query.get_columns()] == ["ball"]
    project = query.get_variant()
    assert isinstance(project, semantics.QueryProject)
    assert [column.get_name() for column in project.get_columns()] == ["ball"]
    atom = project.get_arg().get_variant()
    assert isinstance(atom, semantics.QueryAtomicStateFluent)
    assert not hasattr(atom, "get_polarity")
    assert not hasattr(semantics.QueryAtomicStateFluentData(), "polarity")
    assert hasattr(semantics.QueryAtomicGoalFluent, "get_polarity")
    assert hasattr(semantics.QueryAtomicGoalFluentData(), "polarity")
    data = semantics.QueryData()
    data.variant = atom.get_index()
    assert data.variant == atom.get_index()
    assert [column.get_name() for column in atom.get_columns()] == ["ball", "room"]
    assert list(semantics.QueryProject.Fields.__members__) == ["columns", "arg"]
    assert list(semantics.QueryColumn.Fields.__members__) == ["name"]

    dictionaries = Dictionaries()
    register_table(dictionaries, semantics.Query, "queries", "q")
    register_table(dictionaries, semantics.QueryProject, "projects", "p")
    register_table(dictionaries, semantics.QueryColumn, "columns", "c")
    register_table(
        dictionaries,
        semantics.QueryAtomicStateFluent,
        "atoms",
        "a",
        project=lambda value: {"predicate": value.get_predicate().get_name()},
    )
    assert serialize(dictionaries, query) == "q0"
    assert table(dictionaries, semantics.QueryProject) == [{"columns": ["c0"], "arg": "q1"}]
    assert table(dictionaries, semantics.QueryColumn) == [{"name": "ball"}]
    assert table(dictionaries, semantics.QueryAtomicStateFluent) == [{"predicate": "at"}]


_NAMED_QUERY_PROGRAM = """(:program (:entry root)
  (:module (:symbol root) (:arguments) (:registers)
    (:entry m0) (:memory m0 m1)
    (:features
      (:concept (:symbol candidates)
        (:expression (c_project ball (q_atomic_state "at" (ball room)))))
      (:role (:symbol locations)
        (:expression (r_project room ball (q_atomic_state "at" (ball room))))))
    (:rules
      (:rule (:symbol call) (:expression
        (:source-memory m0) (:target-memory m1)
        (:call (:conditions) (:callee worker) (:arguments candidates))))))
  (:module (:symbol worker) (:arguments (:concept X)) (:registers (:role R))
    (:entry m0) (:memory m0)
    (:features
      (:concept (:symbol matching)
        (:expression (c_project ball (q_join
          (q_concept ball (c_argument X))
          (q_role (ball room) (r_register R))))))
      (:numerical (:symbol count)
        (:expression (n_count (q_role (ball room) (r_register R))))))
    (:rules)))"""


def test_query_program_round_trip_preserves_named_references(gripper_planning_domain: PlanningDomain) -> None:
    context = DomainContext(gripper_planning_domain)
    program = parse_program(_NAMED_QUERY_PROGRAM, gripper_planning_domain, context.ext_repository)
    formatted = str(program)
    fresh_context = DomainContext(gripper_planning_domain)
    reparsed = parse_program(formatted, gripper_planning_domain, fresh_context.ext_repository)

    assert str(reparsed) == formatted
    assert reparsed.get_entry_module().get_name() == "root"
    assert len(reparsed.get_entry_module().get_memory_transitions()) == 1
    assert str(reparsed.get_entry_module().get_role_features()[0].get_expression()) == (
        '(r_project room ball (q_atomic_state "at" (ball room)))'
    )
    worker = next(module for module in reparsed.get_modules() if module.get_name() == "worker")
    assert [argument.get_name() for argument in worker.get_concept_arguments()] == ["X"]
    assert [register.get_name() for register in worker.get_role_registers()] == ["R"]
    assert str(worker.get_concept_features()[0].get_expression()) == (
        "(c_project ball (q_join (q_concept ball (c_argument X)) (q_role (ball room) (r_register R))))"
    )
    assert str(worker.get_numerical_features()[0].get_expression()) == ("(n_count (q_role (ball room) (r_register R)))")
    query = _concept_query(worker.get_concept_features()[0].get_expression())
    assert [column.get_name() for column in query.get_columns()] == ["ball", "room"]
    join = query.get_variant()
    assert isinstance(join, semantics.QueryJoin)
    assert [column.get_name() for column in join.get_columns()] == ["ball", "room"]
    assert list(semantics.QueryJoin.Fields.__members__) == ["columns", "lhs", "rhs"]


@pytest.mark.parametrize(
    ("reference", "missing", "kind"),
    [
        ("c_argument X", "c_argument missing", "concept argument"),
        ("r_register R", "r_register missing", "role register"),
    ],
)
def test_query_program_rejects_undefined_named_references(
    gripper_planning_domain: PlanningDomain, reference: str, missing: str, kind: str
) -> None:
    source = _NAMED_QUERY_PROGRAM.replace(reference, missing)
    context = DomainContext(gripper_planning_domain)
    with pytest.raises(UndefinedSymbolError, match=f"Undefined {kind}: missing") as raised:
        parse_program(source, gripper_planning_domain, context.ext_repository)
    assert raised.value.diagnostic.location is not None
    assert raised.value.diagnostic.location.begin == source.index("missing")


@pytest.mark.parametrize(
    ("query", "expected"),
    [
        ('(q_atomic_state "at" (ball room))', 2),
        ('(q_project (room) (q_atomic_state "at" (ball room)))', 1),
        ('(q_project () (q_atomic_state "at" (ball room)))', 1),
        ('(q_select_equal ball room (q_atomic_state "at" (ball room)))', 0),
        ('(q_rename (room ball) (q_atomic_state "at" (ball room)))', 2),
        ('(q_join (q_atomic_state "at" (ball room)) (q_concept room (c_atomic_state "at-robby")))', 2),
        ('(q_union (q_atomic_state "at" (ball room)) (q_atomic_state "at" (ball room)))', 2),
        ('(q_difference (q_atomic_state "at" (ball room)) (q_atomic_state "at" (ball room)))', 0),
    ],
)
def test_query_count_evaluation(
    gripper_planning_domain: PlanningDomain,
    ground_gripper_search_context: GroundTaskSearchContext,
    query: str,
    expected: int,
) -> None:
    domain = DomainContext(gripper_planning_domain)
    search = ground_gripper_search_context
    task = GroundTaskContext(domain, search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    storage = semantics.EvaluationStorage(task.dl_denotation_repository)
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, storage)
    owner = _sketch(f"(n_count {query})", "numerical", gripper_planning_domain, domain)
    expression = owner.get_numerical_features()[0].get_expression()
    assert expression.evaluate(context).get() == expected
    assert str(expression) == f"(n_count {query})"


_AT = '(q_atomic_state "at" (ball room))'


@pytest.mark.parametrize(
    ("expression", "expected"),
    [
        (
            f'(n_distance (q_concept ball (c_atomic_state "ball")) {_AT} (q_concept room (c_atomic_state "at-robby")))',
            1,
        ),
        (f'(n_distance (c_atomic_state "ball") {_AT} (c_atomic_state "at-robby"))', 1),
        ('(n_distance (q_concept ball (c_atomic_state "ball")) (r_atomic_state "at") (c_atomic_state "at-robby"))', 1),
        (f'(n_distance (c_atomic_state "ball") {_AT} (c_atomic_state "ball"))', 0),
        (f'(n_distance (c_atomic_state "room") {_AT} (c_atomic_state "ball"))', 2**32 - 1),
    ],
)
def test_query_distance_evaluation(
    gripper_planning_domain: PlanningDomain,
    ground_gripper_search_context: GroundTaskSearchContext,
    expression: str,
    expected: int,
) -> None:
    domain = DomainContext(gripper_planning_domain)
    search = ground_gripper_search_context
    task = GroundTaskContext(domain, search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    storage = semantics.EvaluationStorage(task.dl_denotation_repository)
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, storage)
    owner = _sketch(expression, "numerical", gripper_planning_domain, domain)
    feature = owner.get_numerical_features()[0].get_expression()
    assert feature.evaluate(context).get() == expected
    assert str(feature) == expression


def test_concept_and_role_projection_bindings(
    gripper_planning_domain: PlanningDomain, ground_gripper_search_context: GroundTaskSearchContext
) -> None:
    domain = DomainContext(gripper_planning_domain)
    search = ground_gripper_search_context
    task = GroundTaskContext(domain, search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    storage = semantics.EvaluationStorage(task.dl_denotation_repository)
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, storage)
    atom = '(q_atomic_state "at" (ball room))'
    owner = parse_sketch(
        "".join(
            (
                "(:sketch (:features ",
                f"(:numerical (:symbol balls) (:expression (n_count (c_project ball {atom})))) ",
                f"(:numerical (:symbol reverse) (:expression (n_count (r_project room ball {atom})))) ",
                f"(:boolean (:symbol any) (:expression (b_nonempty {atom})))) (:rules))",
            )
        ),
        gripper_planning_domain,
        domain.base_repository,
    )
    features = {feature.get_symbol(): feature.get_expression() for feature in owner.get_numerical_features()}
    concept = _numerical_arg(features["balls"])
    role = _numerical_arg(features["reverse"])
    assert isinstance(concept, semantics.Concept)
    assert isinstance(role, semantics.Role)
    assert isinstance(concept.get_variant(), semantics.ConceptProject)
    assert not hasattr(semantics.ConceptTop, "get_columns")
    assert not hasattr(semantics.RoleUniversal, "get_columns")
    assert not hasattr(semantics.ConceptProject, "get_plan")
    assert not hasattr(semantics.RoleProject, "get_plan")
    assert isinstance(role.get_variant(), semantics.RoleProject)
    assert concept.syntactic_complexity() == role.syntactic_complexity() == 2
    assert {value.get_name() for value in concept.evaluate(context)} == {"ball1", "ball2"}
    assert {(lhs.get_name(), rhs.get_name()) for lhs, rhs in role.evaluate(context)} == {
        ("rooma", "ball1"),
        ("rooma", "ball2"),
    }
    assert str(concept) == f"(c_project ball {atom})"
    assert str(role) == f"(r_project room ball {atom})"

    repository = task.dl_denotation_repository
    persistent_caches = semantics.DenotationCaches()
    retained_context = semantics.GroundStateEvaluationContext(
        state, task.dl_builder, persistent_caches, repository, storage
    )
    saved_concept = concept.evaluate(retained_context)
    saved_role = role.evaluate(retained_context)
    saved_count = features["balls"].evaluate(retained_context)
    saved_boolean = owner.get_boolean_features()[0].get_expression().evaluate(retained_context)
    storage.reset_all()
    assert concept.evaluate(retained_context) == saved_concept
    assert role.evaluate(retained_context) == saved_role
    assert features["balls"].evaluate(retained_context) == saved_count
    assert owner.get_boolean_features()[0].get_expression().evaluate(retained_context) == saved_boolean
    assert {value.get_name() for value in saved_concept} == {"ball1", "ball2"}
    assert {(lhs.get_name(), rhs.get_name()) for lhs, rhs in saved_role} == {
        ("rooma", "ball1"),
        ("rooma", "ball2"),
    }
    assert saved_count.get() == 2
    assert saved_boolean.get() is True


@pytest.mark.parametrize("owner_kind", ["query", "concrete", "concept", "role"])
def test_query_column_retains_owner_after_list_and_repository_scope(
    gripper_planning_domain: PlanningDomain, owner_kind: str
) -> None:
    def column_after_owner_scope() -> tuple[semantics.QueryColumn, str]:
        factory = semantics.ConstructorRepositoryFactory()
        repository = factory.create(gripper_planning_domain)
        column_indices: list[semantics.QueryColumnIndex] = []
        for name in ("left", "right"):
            data = semantics.QueryColumnData()
            data.name = name
            column_indices.append(repository.insert(data)[0].get_index())

        role_data = semantics.RoleData()
        role_data.variant = repository.insert(semantics.RoleUniversalData())[0].get_index()
        role = repository.insert(role_data)[0]
        concrete_data = semantics.QueryRoleData()
        concrete_data.arg = role.get_index()
        concrete_data.columns = column_indices
        concrete = repository.insert(concrete_data)[0]
        query_data = semantics.QueryData()
        query_data.variant = concrete.get_index()
        query = repository.insert(query_data)[0]

        if owner_kind in ("concept", "role"):
            data = semantics.ConceptProjectData() if owner_kind == "concept" else semantics.RoleProjectData()
            data.arg = query.get_index()
            data.columns = column_indices[1:] if owner_kind == "concept" else column_indices[::-1]
            owner = repository.insert(data)[0]
            expected = ["right"] if owner_kind == "concept" else ["right", "left"]
        else:
            owner = query if owner_kind == "query" else concrete
            expected = ["left", "right"]
        columns = owner.get_columns()
        assert isinstance(columns, list)
        assert all(isinstance(column, semantics.QueryColumn) for column in columns)
        assert [column.get_name() for column in columns] == expected
        column = columns[0]
        del columns
        return column, expected[0]

    column, expected = column_after_owner_scope()
    gc.collect()
    assert column.get_name() == expected


def test_native_ext_query_construction_derives_schema_and_evaluates(
    gripper_planning_domain: PlanningDomain, ground_gripper_search_context: GroundTaskSearchContext
) -> None:
    domain = DomainContext(gripper_planning_domain)
    repository = domain.ext_repository.get_dl_repository()
    count = '(n_count (q_join (q_atomic_state "at" (ball room)) (q_concept room (c_atomic_state "at-robby"))))'
    program = parse_program(
        "".join(
            (
                "(:program (:entry root) (:module (:symbol root) (:arguments) (:registers) ",
                f"(:entry m0) (:memory m0) (:features (:numerical (:symbol count) (:expression {count}))) (:rules)))",
            )
        ),
        gripper_planning_domain,
        domain.ext_repository,
    )
    expression = program.get_entry_module().get_numerical_features()[0].get_expression()
    query_arg = _numerical_arg(expression)
    assert isinstance(query_arg, semantics.Query)
    original = query_arg.get_variant()
    assert isinstance(original, semantics.QueryJoin)
    join_data = semantics.QueryJoinData()
    join_data.lhs = original.get_rhs().get_index()
    join_data.rhs = original.get_lhs().get_index()
    joined = repository.insert(join_data)[0]
    assert [column.get_name() for column in joined.get_columns()] == ["room", "ball"]
    query_data = semantics.QueryData()
    query_data.variant = joined.get_index()
    query = repository.insert(query_data)[0]
    count_data = semantics.NumericalCountData()
    count_data.arg = query.get_index()
    count = repository.insert(count_data)[0]
    numerical_data = semantics.NumericalData()
    numerical_data.variant = count.get_index()
    rewritten = repository.insert(numerical_data)[0]

    search = ground_gripper_search_context
    task = GroundTaskContext(domain, search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    storage = semantics.EvaluationStorage(task.dl_denotation_repository)
    arguments = task.dl_denotation_repository.insert(semantics.CallArgumentsData())[0]
    registers = task.dl_denotation_repository.insert(semantics.RegisterValuesData())[0]
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, storage, arguments, registers)
    assert rewritten.evaluate(context).get() == expression.evaluate(context).get() == 2
