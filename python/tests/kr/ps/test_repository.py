# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

from collections.abc import Callable
from typing import Protocol, TypeVar

import pytest
from fixture_utils import read_fixture
from pytyr.formalism.planning import PlanningDomain

from pyrunir.datasets import GroundTaskClass
from pyrunir.kr import DomainContext
from pyrunir.kr.dl import ConstructorRepositoryFactory as DlRepositoryFactory
from pyrunir.kr.ps import base, ext, icp
from pyrunir.kr.uns import dl as uns_dl


def test_base_and_ext_repositories_construct_programmatically(gripper_planning_domain: PlanningDomain) -> None:
    base_repository = base.RepositoryFactory().create(DlRepositoryFactory().create(gripper_planning_domain))
    rule_data = base.RuleData()
    rule_data.symbol = "r"
    rule = base_repository.insert(rule_data)[0]
    assert rule.get_symbol() == "r"
    assert base_repository.insert(rule_data)[0] == rule

    ext_repository = ext.RepositoryFactory().create(DlRepositoryFactory().create(gripper_planning_domain))
    memory_state_data = ext.MemoryStateData()
    memory_state_data.name = "m0"
    memory_state = ext_repository.insert(memory_state_data)[0]
    assert memory_state.get_name() == "m0"
    assert ext_repository.insert(memory_state_data)[0] == memory_state


def test_uns_features_construct_programmatically(gripper_planning_domain: PlanningDomain) -> None:
    repository = DomainContext(gripper_planning_domain).uns_repository
    classifier = uns_dl.parse_classifier(
        read_fixture("kr/uns/positive.classifier"), gripper_planning_domain, repository
    )
    feature = classifier.get_features()[0]
    concrete_data = uns_dl.ConcreteBooleanFeatureData()
    concrete_data.feature = feature.get_expression().get_index()
    concrete_data.symbol = feature.get_symbol()
    concrete = repository.insert(concrete_data)[0]
    assert concrete == feature.get_variant()
    data = uns_dl.BooleanFeatureData()
    data.variant = concrete.get_index()
    assert repository.insert(data)[0] == feature


_FEATURES = """(:features
      (:boolean (:symbol populated) (:expression (b_nonempty (c_top))))
      (:numerical (:symbol count) (:expression (n_count (c_top)))))"""
_MODULE = f"(:module (:symbol main) (:arguments) (:registers) (:entry m0) (:memory m0) {_FEATURES} (:rules)"

FeatureT = TypeVar("FeatureT")
FeatureT_co = TypeVar("FeatureT_co", covariant=True)


class _Observation(Protocol[FeatureT_co]):
    def get_feature(self) -> FeatureT_co: ...


def _check_observation(insert: Callable[[], tuple[_Observation[FeatureT], bool]], feature: FeatureT) -> None:
    """Check that the inserted observation refers to `feature` and that reinserting it returns the same view."""
    observation = insert()[0]
    assert observation.get_feature() == feature
    assert insert()[0] == observation


def _check_base(domain: PlanningDomain) -> None:
    repository = DomainContext(domain).base_repository
    policy = base.dl.parse_sketch(f"(:sketch {_FEATURES} (:rules))", domain, repository)
    (boolean,) = policy.get_boolean_features()
    (numerical,) = policy.get_numerical_features()
    assert boolean.get_feature() == boolean.get_expression()
    assert boolean.get_variant().get_feature() == boolean.get_expression()
    concrete_boolean = repository.insert(
        base.dl.ConcreteBooleanFeatureData(boolean.get_expression().get_index(), boolean.get_symbol())
    )[0]
    assert concrete_boolean == boolean.get_variant()
    assert repository.insert(base.dl.BooleanFeatureData(concrete_boolean.get_index()))[0] == boolean
    assert numerical.get_feature() == numerical.get_expression()
    assert numerical.get_variant().get_feature() == numerical.get_expression()
    concrete_numerical = repository.insert(
        base.dl.ConcreteNumericalFeatureData(numerical.get_expression().get_index(), numerical.get_symbol())
    )[0]
    assert concrete_numerical == numerical.get_variant()
    assert repository.insert(base.dl.NumericalFeatureData(concrete_numerical.get_index()))[0] == numerical

    b, n = boolean.get_index(), numerical.get_index()
    _check_observation(lambda: repository.insert(base.dl.PositiveBooleanConditionData(b)), boolean)
    _check_observation(lambda: repository.insert(base.dl.NegativeBooleanConditionData(b)), boolean)
    _check_observation(lambda: repository.insert(base.dl.EqualZeroNumericalConditionData(n)), numerical)
    _check_observation(lambda: repository.insert(base.dl.GreaterZeroNumericalConditionData(n)), numerical)
    _check_observation(lambda: repository.insert(base.dl.PositiveBooleanEffectData(b)), boolean)
    _check_observation(lambda: repository.insert(base.dl.NegativeBooleanEffectData(b)), boolean)
    _check_observation(lambda: repository.insert(base.dl.UnchangedBooleanEffectData(b)), boolean)
    _check_observation(lambda: repository.insert(base.dl.IncreasesNumericalEffectData(n)), numerical)
    _check_observation(lambda: repository.insert(base.dl.DecreasesNumericalEffectData(n)), numerical)
    _check_observation(lambda: repository.insert(base.dl.UnchangedNumericalEffectData(n)), numerical)


def _check_ext(domain: PlanningDomain) -> None:
    repository = DomainContext(domain).ext_repository
    policy = ext.dl.parse_module(f"{_MODULE})", domain, repository)
    (boolean,) = policy.get_boolean_features()
    (numerical,) = policy.get_numerical_features()
    assert boolean.get_feature() == boolean.get_expression()
    assert boolean.get_variant().get_feature() == boolean.get_expression()
    concrete_boolean = repository.insert(
        ext.dl.ConcreteBooleanFeatureData(boolean.get_expression().get_index(), boolean.get_symbol())
    )[0]
    assert concrete_boolean == boolean.get_variant()
    assert repository.insert(ext.dl.BooleanFeatureData(concrete_boolean.get_index()))[0] == boolean
    assert numerical.get_feature() == numerical.get_expression()
    assert numerical.get_variant().get_feature() == numerical.get_expression()
    concrete_numerical = repository.insert(
        ext.dl.ConcreteNumericalFeatureData(numerical.get_expression().get_index(), numerical.get_symbol())
    )[0]
    assert concrete_numerical == numerical.get_variant()
    assert repository.insert(ext.dl.NumericalFeatureData(concrete_numerical.get_index()))[0] == numerical

    b, n = boolean.get_index(), numerical.get_index()
    _check_observation(lambda: repository.insert(ext.dl.PositiveBooleanConditionData(b)), boolean)
    _check_observation(lambda: repository.insert(ext.dl.NegativeBooleanConditionData(b)), boolean)
    _check_observation(lambda: repository.insert(ext.dl.EqualZeroNumericalConditionData(n)), numerical)
    _check_observation(lambda: repository.insert(ext.dl.GreaterZeroNumericalConditionData(n)), numerical)
    _check_observation(lambda: repository.insert(ext.dl.PositiveBooleanEffectData(b)), boolean)
    _check_observation(lambda: repository.insert(ext.dl.NegativeBooleanEffectData(b)), boolean)
    _check_observation(lambda: repository.insert(ext.dl.UnchangedBooleanEffectData(b)), boolean)
    _check_observation(lambda: repository.insert(ext.dl.IncreasesNumericalEffectData(n)), numerical)
    _check_observation(lambda: repository.insert(ext.dl.DecreasesNumericalEffectData(n)), numerical)
    _check_observation(lambda: repository.insert(ext.dl.UnchangedNumericalEffectData(n)), numerical)


def _check_icp(domain: PlanningDomain) -> None:
    repository = DomainContext(domain).icp_repository
    policy = icp.dl.parse_module(f"{_MODULE} (:reset-spo))", domain, repository)
    (boolean,) = policy.get_boolean_features()
    (numerical,) = policy.get_numerical_features()
    assert boolean.get_feature() == boolean.get_expression()
    assert boolean.get_variant().get_feature() == boolean.get_expression()
    concrete_boolean = repository.insert(
        icp.dl.ConcreteBooleanFeatureData(boolean.get_expression().get_index(), boolean.get_symbol())
    )[0]
    assert concrete_boolean == boolean.get_variant()
    assert repository.insert(icp.dl.BooleanFeatureData(concrete_boolean.get_index()))[0] == boolean
    assert numerical.get_feature() == numerical.get_expression()
    assert numerical.get_variant().get_feature() == numerical.get_expression()
    concrete_numerical = repository.insert(
        icp.dl.ConcreteNumericalFeatureData(numerical.get_expression().get_index(), numerical.get_symbol())
    )[0]
    assert concrete_numerical == numerical.get_variant()
    assert repository.insert(icp.dl.NumericalFeatureData(concrete_numerical.get_index()))[0] == numerical

    b, n = boolean.get_index(), numerical.get_index()
    _check_observation(lambda: repository.insert(icp.dl.PositiveBooleanConditionData(b)), boolean)
    _check_observation(lambda: repository.insert(icp.dl.NegativeBooleanConditionData(b)), boolean)
    _check_observation(lambda: repository.insert(icp.dl.EqualZeroNumericalConditionData(n)), numerical)
    _check_observation(lambda: repository.insert(icp.dl.GreaterZeroNumericalConditionData(n)), numerical)
    _check_observation(lambda: repository.insert(icp.dl.PositiveBooleanEffectData(b)), boolean)
    _check_observation(lambda: repository.insert(icp.dl.NegativeBooleanEffectData(b)), boolean)
    _check_observation(lambda: repository.insert(icp.dl.UnchangedBooleanEffectData(b)), boolean)
    _check_observation(lambda: repository.insert(icp.dl.IncreasesNumericalEffectData(n)), numerical)
    _check_observation(lambda: repository.insert(icp.dl.DecreasesNumericalEffectData(n)), numerical)
    _check_observation(lambda: repository.insert(icp.dl.UnchangedNumericalEffectData(n)), numerical)


@pytest.mark.parametrize("check", [_check_base, _check_ext, _check_icp], ids=["base", "ext", "icp"])
def test_features_and_observations_have_consistent_accessors(
    check: Callable[[PlanningDomain], None], gripper_planning_domain: PlanningDomain
) -> None:
    check(gripper_planning_domain)


def test_ext_module_role_features_and_program_modules_construct_programmatically(
    gripper_planning_domain: PlanningDomain,
) -> None:
    repository = DomainContext(gripper_planning_domain).ext_repository
    parsed_module = ext.dl.parse_module(
        """(:module (:symbol main) (:arguments) (:registers) (:entry m0) (:memory m0)
          (:features (:role (:symbol carrying) (:expression (r_atomic_state "carry")))) (:rules))""",
        gripper_planning_domain,
        repository,
    )
    (role,) = parsed_module.get_role_features()
    module_data = ext.ModuleData()
    module_data.symbol = parsed_module.get_symbol().get_index()
    module_data.role_features = [role.get_index()]
    module_data.entry_memory_state = parsed_module.get_entry_memory_state().get_index()
    module_data.memory_states = [state.get_index() for state in parsed_module.get_memory_states()]
    module = repository.insert(module_data)[0]
    assert module == parsed_module
    assert list(module.get_role_features()) == [role]

    program_data = ext.ProgramData()
    program_data.entry_module = module.get_index()
    program_data.modules = [module.get_index()]
    program = repository.insert(program_data)[0]
    assert program.get_entry_module() == module
    assert list(program.get_modules()) == [module]
    assert ext.dl.parse_program(str(program), gripper_planning_domain, repository) == program


def test_data_rich_comparison_is_bound() -> None:
    first = base.RuleData()
    first.symbol = "a"
    first_copy = base.RuleData()
    first_copy.symbol = "a"
    second = base.RuleData()
    second.symbol = "b"
    second_copy = base.RuleData()
    second_copy.symbol = "b"

    assert first == first_copy
    assert first != second
    assert first < second
    assert first <= first_copy
    assert second > first
    assert second >= second_copy
    with pytest.raises(TypeError):
        hash(first)


def test_task_class_rich_comparison_is_bound() -> None:
    first = GroundTaskClass()
    second = GroundTaskClass()

    assert first == second
    assert first <= second
    assert first >= second
    with pytest.raises(TypeError):
        hash(first)
