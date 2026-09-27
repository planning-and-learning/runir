import pytest

from fixture_utils import read_fixture
from pyrunir.datasets import GroundTaskClass
from pyrunir.kr import DomainContext
from pyrunir.kr.dl.base.semantics import ConstructorRepositoryFactory as BaseDlRepositoryFactory
from pyrunir.kr.dl.ext import ConstructorRepositoryFactory as ExtDlRepositoryFactory
from pyrunir.kr.ps import base, ext, icp
from pyrunir.kr.uns import dl as uns_dl
from pytyr.formalism.planning import PlanningDomain


def test_base_and_ext_repositories_construct_programmatically(gripper_planning_domain: PlanningDomain):
    base_repository = base.RepositoryFactory().create(BaseDlRepositoryFactory().create(gripper_planning_domain))
    rule_data = base.RuleData()
    rule_data.symbol = "r"
    rule = base_repository.get_or_create(rule_data)
    assert rule.get_symbol() == "r"
    assert base_repository.get_or_create(rule_data) == rule

    ext_repository = ext.RepositoryFactory().create(ExtDlRepositoryFactory().create(gripper_planning_domain))
    memory_state_data = ext.MemoryStateData()
    memory_state_data.name = "m0"
    memory_state = ext_repository.get_or_create(memory_state_data)
    assert memory_state.get_name() == "m0"
    assert ext_repository.get_or_create(memory_state_data) == memory_state


def test_uns_features_construct_programmatically(gripper_planning_domain: PlanningDomain):
    repository = DomainContext(gripper_planning_domain).uns_repository
    classifier = uns_dl.parse_classifier(read_fixture("kr/uns/positive.classifier"), gripper_planning_domain, repository)
    feature = classifier.get_features()[0]
    concrete_data = uns_dl.ConcreteBooleanFeatureData()
    concrete_data.feature = feature.get_expression().get_index()
    concrete_data.symbol = feature.get_symbol()
    concrete = repository.get_or_create(concrete_data)
    assert concrete == feature.get_variant()
    data = uns_dl.BooleanFeatureData()
    data.variant = concrete.get_index()
    assert repository.get_or_create(data) == feature


@pytest.mark.parametrize("family", [base, ext, icp], ids=["base", "ext", "icp"])
def test_features_and_observations_have_consistent_accessors(family, gripper_planning_domain: PlanningDomain):
    domain_context = DomainContext(gripper_planning_domain)
    family_name = family.__name__.rsplit(".", 1)[-1]
    repository = getattr(domain_context, f"{family_name}_repository")
    features = """(:features
      (:boolean (:symbol populated) (:expression (b_nonempty (c_top))))
      (:numerical (:symbol count) (:expression (n_count (c_top)))))"""
    if family is base:
        policy = base.dl.parse_sketch(f"(:sketch {features} (:rules))", gripper_planning_domain, repository)
    else:
        reset = "(:reset-spo)" if family is icp else ""
        policy = family.dl.parse_module(
            f"(:module (:symbol main) (:arguments) (:registers) (:entry m0) (:memory m0) {features} (:rules) {reset})",
            gripper_planning_domain,
            repository,
        )
    boolean, = policy.get_boolean_features()
    numerical, = policy.get_numerical_features()
    for feature, name in ((boolean, "BooleanFeature"), (numerical, "NumericalFeature")):
        assert feature.get_feature() == feature.get_expression()
        assert feature.get_variant().get_feature() == feature.get_expression()
        concrete_data = getattr(family.dl, f"Concrete{name}Data")()
        concrete_data.feature = feature.get_expression().get_index()
        concrete_data.symbol = feature.get_symbol()
        concrete = repository.get_or_create(concrete_data)
        assert concrete == feature.get_variant()
        feature_data = getattr(family.dl, f"{name}Data")()
        feature_data.variant = concrete.get_index()
        assert repository.get_or_create(feature_data) == feature

    for name, feature in (
        ("PositiveBooleanCondition", boolean),
        ("NegativeBooleanCondition", boolean),
        ("EqualZeroNumericalCondition", numerical),
        ("GreaterZeroNumericalCondition", numerical),
        ("PositiveBooleanEffect", boolean),
        ("NegativeBooleanEffect", boolean),
        ("UnchangedBooleanEffect", boolean),
        ("IncreasesNumericalEffect", numerical),
        ("DecreasesNumericalEffect", numerical),
        ("UnchangedNumericalEffect", numerical),
    ):
        data = getattr(family.dl, f"{name}Data")()
        data.feature = feature.get_index()
        observation = repository.get_or_create(data)
        assert observation.get_feature() == feature
        assert repository.get_or_create(data) == observation


def test_ext_module_role_features_and_program_modules_construct_programmatically(gripper_planning_domain: PlanningDomain):
    repository = DomainContext(gripper_planning_domain).ext_repository
    parsed_module = ext.dl.parse_module(
        """(:module (:symbol main) (:arguments) (:registers) (:entry m0) (:memory m0)
          (:features (:role (:symbol carrying) (:expression (r_atomic_state "carry")))) (:rules))""",
        gripper_planning_domain,
        repository,
    )
    role, = parsed_module.get_role_features()
    module_data = ext.ModuleData()
    module_data.symbol = parsed_module.get_symbol().get_index()
    module_data.role_features = [role.get_index()]
    module_data.entry_memory_state = parsed_module.get_entry_memory_state().get_index()
    module_data.memory_states = [state.get_index() for state in parsed_module.get_memory_states()]
    module = repository.get_or_create(module_data)
    assert module == parsed_module
    assert list(module.get_role_features()) == [role]

    program_data = ext.ProgramData()
    program_data.entry_module = module.get_index()
    program_data.modules = [module.get_index()]
    program = repository.get_or_create(program_data)
    assert program.get_entry_module() == module
    assert list(program.get_modules()) == [module]
    assert ext.dl.parse_program(str(program), gripper_planning_domain, repository) == program


def test_data_rich_comparison_is_bound():
    first = base.RuleData()
    first.symbol = "a"
    second = base.RuleData()
    second.symbol = "b"

    assert first == first
    assert first != second
    assert first < second
    assert first <= first
    assert second > first
    assert second >= second
    with pytest.raises(TypeError):
        hash(first)


def test_task_class_rich_comparison_is_bound():
    first = GroundTaskClass()
    second = GroundTaskClass()

    assert first == second
    assert first <= second
    assert first >= second
    with pytest.raises(TypeError):
        hash(first)
