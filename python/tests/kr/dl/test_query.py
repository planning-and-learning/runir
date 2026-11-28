# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import gc

import pytest
from pytyr.formalism import Object, PlanningDomain
from pyyggdrasil import formalism
from pyyggdrasil.formalism import ObjectIndex
from pyyggdrasil.serialization import Dictionaries
from relation_utils import rename, values

from pyrunir.datasets import GroundTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext, UndefinedSymbolError
from pyrunir.kr.dl import semantics
from pyrunir.kr.ps.base import Sketch
from pyrunir.kr.ps.base.dl import parse_sketch
from pyrunir.kr.ps.ext.dl import parse_program
from pyrunir.kr.uns.dl import parse_classifier


def _row_values(rows: formalism.RelationSchema) -> list[int]:
    result: list[int] = []
    for row in rows.rows():
        for value in values(row):
            assert isinstance(value, int)
            result.append(value)
    return result


def _boolean_arg(
    expression: semantics.ConstructorBoolean,
) -> semantics.ConstructorConcept | semantics.ConstructorRole | semantics.Query:
    variant = expression.variant()
    assert isinstance(variant, semantics.BooleanSemanticNonempty)
    return variant.operand()


def _numerical_arg(
    expression: semantics.ConstructorNumerical,
) -> semantics.ConstructorConcept | semantics.ConstructorRole | semantics.Query:
    variant = expression.variant()
    assert isinstance(variant, semantics.NumericalSemanticCount)
    return variant.operand()


def _concept_query(expression: semantics.ConstructorConcept) -> semantics.Query:
    variant = expression.variant()
    assert isinstance(variant, semantics.QueryProjectionConcept)
    return variant.operand()


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
        expression = owner.boolean_features()[0].variant().feature()
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
        expression = owner.entry_mod().boolean_features()[0].variant().feature()
    else:
        owner = parse_classifier(
            f"(:classifier (:symbol root) (:features {feature}) (:expression (or (and value))))",
            gripper_planning_domain,
            context.uns_repository,
        )
        expression = owner.features()[0].variant().feature()

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
    assert semantics.evaluate(expression, evaluation_context).value() is True
    persistent_caches = semantics.DenotationCaches()
    repository = task.dl_denotation_repository
    retained_context = semantics.GroundStateEvaluationContext(
        state, task.dl_builder, persistent_caches, repository, storage, *bindings
    )
    retained = semantics.evaluate(expression, retained_context)
    query = _boolean_arg(expression)
    assert isinstance(query, semantics.Query)
    retained_query = semantics.evaluate(query, retained_context)
    relations = task.dl_denotation_repository.get_relation_repository()
    assert isinstance(relations, formalism.RelationRepository)
    assert isinstance(retained_query, formalism.RelationSchema)
    assert isinstance(retained_query.index(), formalism.RelationSchemaIndex)
    assert all(isinstance(row, formalism.RowSchema) for row in retained_query.rows())
    assert all(
        isinstance(repository.get_object(ObjectIndex(object_)), Object) for object_ in _row_values(retained_query)
    )
    assert rename(relations, retained_query, [column.label for column in retained_query.columns()]) == retained_query
    retained_rows = {values(row) for row in retained_query.rows()}
    storage.reset_dynamic()
    assert semantics.evaluate(expression, evaluation_context).value() is True
    storage.reset_all()
    assert retained.value() is True
    assert {values(row) for row in retained_query.rows()} == retained_rows
    assert semantics.evaluate(expression, evaluation_context).value() is True
    ordinary_query = semantics.evaluate(query, evaluation_context)
    del evaluation_context, retained_context, persistent_caches, storage, repository, relations, task, state, bindings
    gc.collect()
    assert retained.value() is True
    assert {values(row) for row in retained_query.rows()} == retained_rows
    assert {values(row) for row in ordinary_query.rows()} == retained_rows

    formatted = str(owner)
    assert source in formatted
    fresh_context = DomainContext(gripper_planning_domain)
    if family == "base":
        reparsed = parse_sketch(formatted, gripper_planning_domain, fresh_context.base_repository)
        assert len(reparsed.rules()) == 1
    elif family == "ext":
        reparsed = parse_program(formatted, gripper_planning_domain, fresh_context.ext_repository)
    else:
        reparsed = parse_classifier(formatted, gripper_planning_domain, fresh_context.uns_repository)
    assert str(reparsed) == formatted

    assert str(expression) == source
    assert expression.syntactic_complexity() == 3
    assert [column.name() for column in query.variant().columns()] == ["ball"]
    project = query.variant()
    assert isinstance(project, semantics.QueryProject)
    assert [column.name() for column in project.columns()] == ["ball"]
    atom = project.operand().variant()
    assert isinstance(atom, semantics.QueryAtomicStateFluent)
    data = semantics.QueryData()
    data.variant = atom.index()
    assert data.variant == atom.index()
    assert [column.name() for column in atom.columns()] == ["ball", "room"]
    assert list(semantics.QueryProject.Fields.__members__) == ["operand", "columns", "plan"]
    assert list(semantics.QueryColumn.Fields.__members__) == ["name"]

    dictionaries = Dictionaries()
    dictionaries.register_table(semantics.Query, "queries", "q")
    # Its operand and columns: not the plan the repository derives from them (a table selects its fields).
    dictionaries.register_table(semantics.QueryProject, "projects", "p", fields=("operand", "columns"))
    dictionaries.register_table(semantics.QueryColumn, "columns", "c")
    dictionaries.register_table(
        semantics.QueryAtomicStateFluent,
        "atoms",
        "a",
        project=lambda value: {"predicate": value.predicate().name()},
    )
    assert dictionaries.serialize(query) == "q0"
    assert dictionaries.table(semantics.QueryProject) == [{"columns": ["c0"], "operand": "q1"}]
    assert dictionaries.table(semantics.QueryColumn) == [{"name": "ball"}]
    assert dictionaries.table(semantics.QueryAtomicStateFluent) == [{"predicate": "at"}]


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
    assert reparsed.entry_mod().symbol().name() == "root"
    assert len(reparsed.entry_mod().memory_transitions()) == 1
    assert str(reparsed.entry_mod().role_features()[0].variant().feature()) == (
        '(r_project room ball (q_atomic_state "at" (ball room)))'
    )
    worker = next(module for module in reparsed.modules() if module.symbol().name() == "worker")
    assert [argument.name() for argument in worker.concept_arguments()] == ["X"]
    assert [register.name() for register in worker.role_registers()] == ["R"]
    assert str(worker.concept_features()[0].variant().feature()) == (
        "(c_project ball (q_join (q_concept ball (c_argument X)) (q_role (ball room) (r_register R))))"
    )
    assert str(worker.numerical_features()[0].variant().feature()) == ("(n_count (q_role (ball room) (r_register R)))")
    query = _concept_query(worker.concept_features()[0].variant().feature())
    assert [column.name() for column in query.variant().columns()] == ["ball", "room"]
    join = query.variant()
    assert isinstance(join, semantics.QueryJoin)
    assert [column.name() for column in join.columns()] == ["ball", "room"]
    assert list(semantics.QueryJoin.Fields.__members__) == ["left_operand", "right_operand", "columns", "plan"]


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
    expression = owner.numerical_features()[0].variant().feature()
    assert semantics.evaluate(expression, context).value() == expected
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
    feature = owner.numerical_features()[0].variant().feature()
    assert semantics.evaluate(feature, context).value() == expected
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
    features = {feature.variant().symbol(): feature.variant().feature() for feature in owner.numerical_features()}
    concept = _numerical_arg(features["balls"])
    role = _numerical_arg(features["reverse"])
    assert isinstance(concept, semantics.ConstructorConcept)
    assert isinstance(role, semantics.ConstructorRole)
    assert isinstance(concept.variant(), semantics.QueryProjectionConcept)
    assert isinstance(role.variant(), semantics.QueryProjectionRole)
    assert concept.syntactic_complexity() == role.syntactic_complexity() == 2
    assert {value.name() for value in semantics.evaluate(concept, context).views()} == {"ball1", "ball2"}
    assert {(lhs.name(), rhs.name()) for lhs, rhs in semantics.evaluate(role, context).views()} == {
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
    saved_concept = semantics.evaluate(concept, retained_context)
    saved_role = semantics.evaluate(role, retained_context)
    saved_count = semantics.evaluate(features["balls"], retained_context)
    saved_boolean = semantics.evaluate(owner.boolean_features()[0].variant().feature(), retained_context)
    storage.reset_all()
    assert semantics.evaluate(concept, retained_context) == saved_concept
    assert semantics.evaluate(role, retained_context) == saved_role
    assert semantics.evaluate(features["balls"], retained_context) == saved_count
    assert semantics.evaluate(owner.boolean_features()[0].variant().feature(), retained_context) == saved_boolean
    assert {value.name() for value in saved_concept.views()} == {"ball1", "ball2"}
    assert {(lhs.name(), rhs.name()) for lhs, rhs in saved_role.views()} == {
        ("rooma", "ball1"),
        ("rooma", "ball2"),
    }
    assert saved_count.value() == 2
    assert saved_boolean.value() is True


@pytest.mark.parametrize("owner_kind", ["query", "concrete", "concept", "role"])
def test_query_column_retains_owner_after_list_and_repository_scope(
    gripper_planning_domain: PlanningDomain, owner_kind: str
) -> None:
    def column_after_owner_scope() -> tuple[semantics.QueryColumn, str]:
        repository = semantics.ConstructorRepository(gripper_planning_domain)
        column_indices: list[semantics.QueryColumnIndex] = []
        for name in ("left", "right"):
            data = semantics.QueryColumnData()
            data.name = name
            column_indices.append(repository.insert(data)[0].index())

        role_data = semantics.ConstructorRoleData()
        role_data.variant = repository.insert(semantics.RoleSemanticUniversalData())[0].index()
        role = repository.insert(role_data)[0]
        concrete_data = semantics.QueryRoleData()
        concrete_data.operand = role.index()
        concrete_data.columns = column_indices
        concrete = repository.insert(concrete_data)[0]
        query_data = semantics.QueryData()
        query_data.variant = concrete.index()
        query = repository.insert(query_data)[0]

        if owner_kind in ("concept", "role"):
            data = (
                semantics.QueryProjectionConceptData()
                if owner_kind == "concept"
                else semantics.QueryProjectionRoleData()
            )
            data.operand = query.index()
            data.columns = column_indices[1:] if owner_kind == "concept" else column_indices[::-1]
            owner = repository.insert(data)[0]
            expected = ["right"] if owner_kind == "concept" else ["right", "left"]
        else:
            owner = query.variant() if owner_kind == "query" else concrete
            expected = ["left", "right"]
        columns = owner.columns()
        assert isinstance(columns, list)
        assert all(isinstance(column, semantics.QueryColumn) for column in columns)
        assert [column.name() for column in columns] == expected
        column = columns[0]
        del columns
        return column, expected[0]

    column, expected = column_after_owner_scope()
    gc.collect()
    assert column.name() == expected


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
    expression = program.entry_mod().numerical_features()[0].variant().feature()
    query_arg = _numerical_arg(expression)
    assert isinstance(query_arg, semantics.Query)
    original = query_arg.variant()
    assert isinstance(original, semantics.QueryJoin)
    join_data = semantics.QueryJoinData()
    join_data.left_operand = original.right_operand().index()
    join_data.right_operand = original.left_operand().index()
    joined = repository.insert(join_data)[0]
    assert [column.name() for column in joined.columns()] == ["room", "ball"]
    query_data = semantics.QueryData()
    query_data.variant = joined.index()
    query = repository.insert(query_data)[0]
    count_data = semantics.NumericalSemanticCountData()
    count_data.operand = query.index()
    count = repository.insert(count_data)[0]
    numerical_data = semantics.ConstructorNumericalData()
    numerical_data.variant = count.index()
    rewritten = repository.insert(numerical_data)[0]

    search = ground_gripper_search_context
    task = GroundTaskContext(domain, search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    storage = semantics.EvaluationStorage(task.dl_denotation_repository)
    arguments = task.dl_denotation_repository.insert(semantics.CallArgumentsData())[0]
    registers = task.dl_denotation_repository.insert(semantics.RegisterValuesData())[0]
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, storage, arguments, registers)
    assert semantics.evaluate(rewritten, context).value() == semantics.evaluate(expression, context).value() == 2
