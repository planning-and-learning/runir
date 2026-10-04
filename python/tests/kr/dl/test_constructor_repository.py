import gc
import sys

import pytest

from pyrunir.kr import DomainContext, GroundTaskContext
from pyrunir.kr.dl import base, ext, uns


@pytest.mark.parametrize("family", [base, ext, uns], ids=["base", "ext", "uns"])
def test_insert_returns_creation_flag_and_retains_owner_on_view(gripper_planning_domain, family):
    def create_view():
        repository = family.semantics.ConstructorRepositoryFactory().create(gripper_planning_domain)
        data = base.semantics.QueryColumnData()
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


@pytest.mark.parametrize("family", [base, uns], ids=["base", "uns"])
@pytest.mark.parametrize("category, leaf, names", [
    ("Concept", "ConceptTop", ["object"]),
    ("Role", "RoleUniversal", ["left", "right"]),
])
def test_repository_constructs_queries_and_semantic_wrappers(
    gripper_planning_domain, ground_gripper_search_context, family, category, leaf, names
):
    semantics = family.semantics
    repository = semantics.ConstructorRepositoryFactory().create(gripper_planning_domain)
    assert family.ConstructorRepositoryFactory is semantics.ConstructorRepositoryFactory
    assert family.ConstructorRepository is semantics.ConstructorRepository

    def create(type_name, **fields):
        data = getattr(semantics, f"{type_name}Data")()
        for field, value in fields.items():
            setattr(data, field, value)
        result = repository.insert(data)[0]
        assert repository.insert(data)[0] == result
        return result

    constructor = create(category, variant=create(leaf).get_index())
    columns = []
    for name in names:
        data = base.semantics.QueryColumnData()
        data.name = name
        columns.append(repository.insert(data)[0].get_index())
    atom = create(f"Query{category}", columns=columns, arg=constructor.get_index())
    query = create("Query", variant=atom.get_index())
    joined = create("QueryJoin", lhs=query.get_index(), rhs=query.get_index())
    assert [column.get_name() for column in joined.get_columns()] == names
    query = create("Query", variant=joined.get_index())
    projected = create(f"{category}Project", columns=columns[::-1], arg=query.get_index())
    assert [column.get_name() for column in projected.get_columns()] == names[::-1]
    projection = create(category, variant=projected.get_index())
    assert projection.get_variant() == projected

    count = create("NumericalCount", arg=projection.get_index())
    numerical = create("Numerical", variant=count.get_index())
    nonempty = create("BooleanNonempty", arg=query.get_index())
    boolean = create("Boolean", variant=nonempty.get_index())
    assert numerical.get_variant() == count
    assert boolean.get_variant() == nonempty

    search = ground_gripper_search_context
    task = GroundTaskContext(DomainContext(gripper_planning_domain), search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    storage = semantics.EvaluationStorage(task.dl_denotation_repository)
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, storage)
    assert numerical.evaluate(context).get() == sum(1 for _ in constructor.evaluate(context))
    assert boolean.evaluate(context).get() is True


@pytest.mark.parametrize("type_name", [
    "ConceptArgument", "RoleArgument", "BooleanArgument", "NumericalArgument",
    "ConceptRegister", "RoleRegister",
])
def test_ext_repository_constructs_references(gripper_planning_domain, type_name):
    assert ext.ConstructorRepositoryFactory is ext.semantics.ConstructorRepositoryFactory
    assert ext.ConstructorRepository is ext.semantics.ConstructorRepository
    repository = ext.semantics.ConstructorRepositoryFactory().create(gripper_planning_domain)
    data = getattr(ext, f"{type_name}Data")()
    data.name = "reference"
    data.identifier = getattr(ext, f"{type_name}Identifier")(3)
    reference = repository.insert(data)[0]
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier
    assert repository.insert(data)[0] == reference
