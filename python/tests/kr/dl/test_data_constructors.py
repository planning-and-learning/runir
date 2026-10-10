from pyrunir.kr.dl import base


def test_value_and_view_constructors_build_equal_data(gripper_planning_domain):
    semantics = base.semantics
    repository = semantics.ConstructorRepositoryFactory().create(gripper_planning_domain)

    top = repository.insert(semantics.ConceptTopData())[0]
    concept = repository.insert(semantics.ConceptData(top.get_index()))[0]
    bot = repository.insert(semantics.ConceptBotData())[0]
    other = repository.insert(semantics.ConceptData(bot.get_index()))[0]

    by_index = semantics.ConceptIntersectionData(concept.get_index(), other.get_index())
    by_view = semantics.ConceptIntersectionData(concept, other)
    assert by_index == by_view
    assert repository.insert(by_index)[0] == repository.insert(by_view)[0]

    column = repository.insert(semantics.QueryColumnData("x"))[0]
    assert column.get_name() == "x"
    query_by_index = semantics.QueryConceptData(concept.get_index(), [column.get_index()])
    query_by_view = semantics.QueryConceptData(concept, [column])
    assert query_by_index == query_by_view


def test_view_variant_constructor_builds_equal_data(gripper_planning_domain):
    semantics = base.semantics
    repository = semantics.ConstructorRepositoryFactory().create(gripper_planning_domain)

    top = repository.insert(semantics.ConceptTopData())[0]
    assert semantics.ConceptData(top) == semantics.ConceptData(top.get_index())
