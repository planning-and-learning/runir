import gc
import pytest

from pyrunir.kr import DomainContext, GroundTaskContext, UndefinedSymbolError
from pyrunir.kr.dl.base import semantics
from pyrunir.kr.dl.ext import semantics as ext_semantics
from pyrunir.kr.dl.uns import semantics as uns_semantics
from pyrunir.kr.ps.base.dl import parse_sketch
from pyrunir.kr.ps.ext.dl import parse_module_program
from pyrunir.kr.uns.dl import parse_classifier
from pyrunir.serialization import register_table, serialize, table
from pyyggdrasil.serialization import Dictionaries


def _sketch(expression, category, domain, context):
    return parse_sketch(
        f"(:sketch (:features (:{category} (:symbol value) "
        f"(:expression {expression}))) (:rules))",
        domain,
        context.base_repository,
    )


@pytest.mark.parametrize("family", ["base", "ext", "uns"])
def test_nested_query_owner_round_trip_bindings_complexity_and_serialization(
    gripper_planning_domain, ground_gripper_search_context, family
):
    context = DomainContext(gripper_planning_domain)
    source = '(b_nonempty (q_project (ball) (q_atomic_state "at" (ball room))))'
    feature = f"(:boolean (:symbol value) (:expression {source}))"
    if family == "base":
        owner = parse_sketch(
            f"(:sketch (:features {feature}) (:rules "
            "(:rule (:symbol keep) (:expression "
            "(:conditions (positive value)) (:effects (unchanged value))))))",
            gripper_planning_domain,
            context.base_repository,
        )
        expression = owner.get_boolean_features()[0].get_expression()
        module = semantics
    elif family == "ext":
        owner = parse_module_program(
            "(:program (:entry root) (:module (:symbol root) (:arguments) (:registers) "
            f"(:entry m0) (:memory m0) (:features {feature}) (:rules)))",
            gripper_planning_domain,
            context.ext_repository,
        )
        expression = owner.get_entry_module().get_boolean_features()[0].get_expression()
        module = ext_semantics
    else:
        owner = parse_classifier(
            f"(:classifier (:symbol root) (:features {feature}) (:expression (or (and value))))",
            gripper_planning_domain,
            context.uns_repository,
        )
        expression = owner.get_features()[0].get_expression()
        module = uns_semantics

    task = GroundTaskContext(context, ground_gripper_search_context)
    state = ground_gripper_search_context.state_repository.get_initial_state(ground_gripper_search_context.axiom_evaluator)
    caches = module.DenotationCaches()
    with pytest.raises(TypeError):
        module.GroundStateEvaluationContext(state, task.dl_builder, task.dl_denotation_repository)
    evaluation_context = module.GroundStateEvaluationContext(state, task.dl_builder, task.dl_denotation_repository, caches)
    assert expression.evaluate(evaluation_context).get() is True
    caches.clear(False)
    assert expression.evaluate(evaluation_context).get() is True
    caches.clear(True)
    assert expression.evaluate(evaluation_context).get() is True

    formatted = str(owner)
    assert source in formatted
    fresh_context = DomainContext(gripper_planning_domain)
    if family == "base":
        reparsed = parse_sketch(formatted, gripper_planning_domain, fresh_context.base_repository)
        assert len(reparsed.get_rules()) == 1
    elif family == "ext":
        reparsed = parse_module_program(formatted, gripper_planning_domain, fresh_context.ext_repository)
    else:
        reparsed = parse_classifier(formatted, gripper_planning_domain, fresh_context.uns_repository)
    assert str(reparsed) == formatted

    assert str(expression) == source
    assert expression.syntactic_complexity() == 3
    query = expression.get_variant().get_arg()
    assert isinstance(query, module.Query)
    assert [column.get_name() for column in query.get_columns()] == ["ball"]
    project = query.get_variant()
    assert isinstance(project, module.QueryProject)
    assert [column.get_name() for column in project.get_columns()] == ["ball"]
    atom = project.get_arg().get_variant()
    assert isinstance(atom, module.QueryAtomicStateFluent)
    assert not hasattr(atom, "get_polarity")
    assert not hasattr(module.QueryAtomicStateFluentData(), "polarity")
    assert hasattr(module.QueryAtomicGoalFluent, "get_polarity")
    assert hasattr(module.QueryAtomicGoalFluentData(), "polarity")
    data = module.QueryData()
    data.variant = atom.get_index()
    assert data.variant == atom.get_index()
    assert [column.get_name() for column in atom.get_columns()] == ["ball", "room"]
    assert list(module.QueryProject.Fields.__members__) == ["columns", "arg"]
    assert list(module.QueryColumn.Fields.__members__) == ["name"]

    dictionaries = Dictionaries()
    register_table(dictionaries, module.Query, "queries", "q")
    register_table(dictionaries, module.QueryProject, "projects", "p")
    register_table(dictionaries, module.QueryColumn, "columns", "c")
    register_table(
        dictionaries, module.QueryAtomicStateFluent, "atoms", "a",
        project=lambda value: {"predicate": value.get_predicate().get_name()},
    )
    assert serialize(dictionaries, query) == "q0"
    assert table(dictionaries, module.QueryProject) == [{"columns": ["c0"], "arg": "q1"}]
    assert table(dictionaries, module.QueryColumn) == [{"name": "ball"}]
    assert table(dictionaries, module.QueryAtomicStateFluent) == [{"predicate": "at"}]


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


def test_query_program_round_trip_preserves_named_references(gripper_planning_domain):
    context = DomainContext(gripper_planning_domain)
    program = parse_module_program(_NAMED_QUERY_PROGRAM, gripper_planning_domain, context.ext_repository)
    formatted = str(program)
    fresh_context = DomainContext(gripper_planning_domain)
    reparsed = parse_module_program(formatted, gripper_planning_domain, fresh_context.ext_repository)

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
        "(c_project ball (q_join (q_concept ball (c_argument X)) "
        "(q_role (ball room) (r_register R))))"
    )
    assert str(worker.get_numerical_features()[0].get_expression()) == (
        "(n_count (q_role (ball room) (r_register R)))"
    )
    query = worker.get_concept_features()[0].get_expression().get_variant().get_arg()
    assert [column.get_name() for column in query.get_columns()] == ["ball", "room"]
    assert [column.get_name() for column in query.get_variant().get_columns()] == ["ball", "room"]
    assert list(ext_semantics.QueryJoin.Fields.__members__) == ["columns", "lhs", "rhs"]


@pytest.mark.parametrize("reference, missing, kind", [
    ("c_argument X", "c_argument missing", "concept argument"),
    ("r_register R", "r_register missing", "role register"),
])
def test_query_program_rejects_undefined_named_references(gripper_planning_domain, reference, missing, kind):
    source = _NAMED_QUERY_PROGRAM.replace(reference, missing)
    context = DomainContext(gripper_planning_domain)
    with pytest.raises(UndefinedSymbolError, match=f"Undefined {kind}: missing") as raised:
        parse_module_program(source, gripper_planning_domain, context.ext_repository)
    assert raised.value.diagnostic.location is not None
    assert raised.value.diagnostic.location.begin == source.index("missing")


@pytest.mark.parametrize("query, expected", [
    ('(q_atomic_state "at" (ball room))', 2),
    ('(q_project (room) (q_atomic_state "at" (ball room)))', 1),
    ('(q_project () (q_atomic_state "at" (ball room)))', 1),
    ('(q_select_equal ball room (q_atomic_state "at" (ball room)))', 0),
    ('(q_rename (room ball) (q_atomic_state "at" (ball room)))', 2),
    ('(q_join (q_atomic_state "at" (ball room)) (q_concept room (c_atomic_state "at-robby")))', 2),
    ('(q_union (q_atomic_state "at" (ball room)) (q_atomic_state "at" (ball room)))', 2),
    ('(q_difference (q_atomic_state "at" (ball room)) (q_atomic_state "at" (ball room)))', 0),
])
def test_query_count_evaluation(gripper_planning_domain, ground_gripper_search_context, query, expected):
    domain = DomainContext(gripper_planning_domain)
    search = ground_gripper_search_context
    task = GroundTaskContext(domain, search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    caches = semantics.DenotationCaches()
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, task.dl_denotation_repository, caches)
    owner = _sketch(f"(n_count {query})", "numerical", gripper_planning_domain, domain)
    expression = owner.get_numerical_features()[0].get_expression()
    assert expression.evaluate(context).get() == expected
    assert str(expression) == f"(n_count {query})"


def test_concept_and_role_projection_bindings(gripper_planning_domain, ground_gripper_search_context):
    domain = DomainContext(gripper_planning_domain)
    search = ground_gripper_search_context
    task = GroundTaskContext(domain, search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    caches = semantics.DenotationCaches()
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, task.dl_denotation_repository, caches)
    atom = '(q_atomic_state "at" (ball room))'
    owner = parse_sketch(
        "(:sketch (:features "
        f"(:numerical (:symbol balls) (:expression (n_count (c_project ball {atom})))) "
        f"(:numerical (:symbol reverse) (:expression (n_count (r_project room ball {atom}))))) (:rules))",
        gripper_planning_domain,
        domain.base_repository,
    )
    features = {feature.get_symbol(): feature.get_expression() for feature in owner.get_numerical_features()}
    concept = features["balls"].get_variant().get_arg()
    role = features["reverse"].get_variant().get_arg()
    assert isinstance(concept.get_variant(), semantics.ConceptProject)
    assert not hasattr(semantics.ConceptTop, "get_columns")
    assert not hasattr(semantics.RoleUniversal, "get_columns")
    assert not hasattr(semantics.ConceptProject, "get_plan")
    assert not hasattr(semantics.RoleProject, "get_plan")
    assert isinstance(role.get_variant(), semantics.RoleProject)
    assert concept.syntactic_complexity() == role.syntactic_complexity() == 2
    assert {value.get_name() for value in concept.evaluate(context)} == {"ball1", "ball2"}
    assert {(lhs.get_name(), rhs.get_name()) for lhs, rhs in role.evaluate(context)} == {
        ("rooma", "ball1"), ("rooma", "ball2"),
    }
    assert str(concept) == f"(c_project ball {atom})"
    assert str(role) == f"(r_project room ball {atom})"


def test_query_column_views_use_caller_owned_repository(gripper_planning_domain):
    context = DomainContext(gripper_planning_domain)

    def columns_after_owner_scope():
        atom = '(q_atomic_state "at" (ball room))'
        source = f'(c_project ball (q_project (ball room) {atom}))'
        concept_owner = _sketch(f"(n_count {source})", "numerical", gripper_planning_domain, context)
        role_owner = _sketch(f"(n_count (r_project room ball {atom}))", "numerical", gripper_planning_domain, context)
        concept = concept_owner.get_numerical_features()[0].get_expression().get_variant().get_arg().get_variant()
        role = role_owner.get_numerical_features()[0].get_expression().get_variant().get_arg().get_variant()
        project = concept.get_arg().get_variant()
        atom_view = project.get_arg().get_variant()
        selected = []
        for view in (concept, role, project, atom_view):
            columns = view.get_columns()
            selected.append(columns[0])
        return selected

    columns = columns_after_owner_scope()
    gc.collect()
    assert [column.get_name() for column in columns] == ["ball", "room", "ball", "ball"]
