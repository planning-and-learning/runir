import pytest

from pyrunir.kr import DomainContext, GroundTaskContext
from pyrunir.kr.dl import base, ext, uns


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
        result = repository.get_or_create(data)
        assert repository.get_or_create(data) == result
        return result

    constructor = create(category, variant=create(leaf).get_index())
    columns = [create("QueryColumn", name=name).get_index() for name in names]
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
    caches = semantics.DenotationCaches(task.dl_denotation_repository)
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, task.dl_denotation_repository, caches)
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
    reference = repository.get_or_create(data)
    assert reference.get_name() == data.name
    assert reference.get_identifier() == data.identifier
    assert repository.get_or_create(data) == reference
