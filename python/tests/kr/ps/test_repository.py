# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

from collections.abc import Callable
from typing import Protocol, TypeVar

import pytest
from fixture_utils import read_fixture, record
from pytyr.formalism import PlanningDomain

from pyrunir.datasets import GroundTaskClass
from pyrunir.kr import DomainContext
from pyrunir.kr.dl import ConstructorRepository as DlRepository
from pyrunir.kr.ps import base, ext, icp
from pyrunir.kr.uns import dl as uns_dl


def test_base_and_ext_repositories_construct_programmatically(gripper_planning_domain: PlanningDomain) -> None:
    base_repository = base.Repository(DlRepository(gripper_planning_domain))
    rule_data = base.RuleBaseData()
    rule_data.symbol = "r"
    rule = base_repository.insert(rule_data)[0]
    assert rule.get_symbol() == "r"
    assert base_repository.insert(rule_data)[0] == rule

    ext_repository = ext.Repository(DlRepository(gripper_planning_domain))
    memory_state_data = ext.MemoryStateExtData()
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
    concrete_data = uns_dl.ConcreteFeatureUnsBooleanData()
    concrete_data.feature = feature.get_variant().get_feature().index()
    concrete_data.symbol = feature.get_variant().get_symbol()
    concrete = repository.insert(concrete_data)[0]
    assert concrete == feature.get_variant()
    data = uns_dl.FeatureUnsBooleanData()
    data.variant = concrete.index()
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
    assert boolean.get_variant().get_feature() == boolean.get_variant().get_feature()
    assert boolean.get_variant().get_feature() == boolean.get_variant().get_feature()
    concrete_boolean = repository.insert(
        record(
            base.dl.ConcreteFeatureBaseBooleanData,
            feature=boolean.get_variant().get_feature().index(),
            symbol=boolean.get_variant().get_symbol(),
        )
    )[0]
    assert concrete_boolean == boolean.get_variant()
    assert repository.insert(record(base.dl.FeatureBaseBooleanData, variant=concrete_boolean.index()))[0] == boolean
    assert numerical.get_variant().get_feature() == numerical.get_variant().get_feature()
    assert numerical.get_variant().get_feature() == numerical.get_variant().get_feature()
    concrete_numerical = repository.insert(
        record(
            base.dl.ConcreteFeatureBaseNumericalData,
            feature=numerical.get_variant().get_feature().index(),
            symbol=numerical.get_variant().get_symbol(),
        )
    )[0]
    assert concrete_numerical == numerical.get_variant()
    assert (
        repository.insert(record(base.dl.FeatureBaseNumericalData, variant=concrete_numerical.index()))[0] == numerical
    )

    b, n = boolean.index(), numerical.index()
    _check_observation(
        lambda: repository.insert(record(base.dl.ConcreteConditionBaseBooleanPositiveData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(base.dl.ConcreteConditionBaseBooleanNegativeData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(base.dl.ConcreteConditionBaseNumericalEqualZeroData, feature=n)), numerical
    )
    _check_observation(
        lambda: repository.insert(record(base.dl.ConcreteConditionBaseNumericalGreaterZeroData, feature=n)), numerical
    )
    _check_observation(
        lambda: repository.insert(record(base.dl.ConcreteEffectBaseBooleanPositiveData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(base.dl.ConcreteEffectBaseBooleanNegativeData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(base.dl.ConcreteEffectBaseBooleanUnchangedData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(base.dl.ConcreteEffectBaseNumericalIncreasesData, feature=n)), numerical
    )
    _check_observation(
        lambda: repository.insert(record(base.dl.ConcreteEffectBaseNumericalDecreasesData, feature=n)), numerical
    )
    _check_observation(
        lambda: repository.insert(record(base.dl.ConcreteEffectBaseNumericalUnchangedData, feature=n)), numerical
    )


def _check_ext(domain: PlanningDomain) -> None:
    repository = DomainContext(domain).ext_repository
    policy = ext.dl.parse_module(f"{_MODULE})", domain, repository)
    (boolean,) = policy.get_boolean_features()
    (numerical,) = policy.get_numerical_features()
    assert boolean.get_variant().get_feature() == boolean.get_variant().get_feature()
    assert boolean.get_variant().get_feature() == boolean.get_variant().get_feature()
    concrete_boolean = repository.insert(
        record(
            ext.dl.ConcreteFeatureExtBooleanData,
            feature=boolean.get_variant().get_feature().index(),
            symbol=boolean.get_variant().get_symbol(),
        )
    )[0]
    assert concrete_boolean == boolean.get_variant()
    assert repository.insert(record(ext.dl.FeatureExtBooleanData, variant=concrete_boolean.index()))[0] == boolean
    assert numerical.get_variant().get_feature() == numerical.get_variant().get_feature()
    assert numerical.get_variant().get_feature() == numerical.get_variant().get_feature()
    concrete_numerical = repository.insert(
        record(
            ext.dl.ConcreteFeatureExtNumericalData,
            feature=numerical.get_variant().get_feature().index(),
            symbol=numerical.get_variant().get_symbol(),
        )
    )[0]
    assert concrete_numerical == numerical.get_variant()
    assert repository.insert(record(ext.dl.FeatureExtNumericalData, variant=concrete_numerical.index()))[0] == numerical

    b, n = boolean.index(), numerical.index()
    _check_observation(
        lambda: repository.insert(record(ext.dl.ConcreteConditionExtBooleanPositiveData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(ext.dl.ConcreteConditionExtBooleanNegativeData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(ext.dl.ConcreteConditionExtNumericalEqualZeroData, feature=n)), numerical
    )
    _check_observation(
        lambda: repository.insert(record(ext.dl.ConcreteConditionExtNumericalGreaterZeroData, feature=n)), numerical
    )
    _check_observation(
        lambda: repository.insert(record(ext.dl.ConcreteEffectExtBooleanPositiveData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(ext.dl.ConcreteEffectExtBooleanNegativeData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(ext.dl.ConcreteEffectExtBooleanUnchangedData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(ext.dl.ConcreteEffectExtNumericalIncreasesData, feature=n)), numerical
    )
    _check_observation(
        lambda: repository.insert(record(ext.dl.ConcreteEffectExtNumericalDecreasesData, feature=n)), numerical
    )
    _check_observation(
        lambda: repository.insert(record(ext.dl.ConcreteEffectExtNumericalUnchangedData, feature=n)), numerical
    )


def _check_icp(domain: PlanningDomain) -> None:
    repository = DomainContext(domain).icp_repository
    policy = icp.dl.parse_module(f"{_MODULE} (:reset-spo))", domain, repository)
    (boolean,) = policy.get_boolean_features()
    (numerical,) = policy.get_numerical_features()
    assert boolean.get_variant().get_feature() == boolean.get_variant().get_feature()
    assert boolean.get_variant().get_feature() == boolean.get_variant().get_feature()
    concrete_boolean = repository.insert(
        record(
            icp.dl.ConcreteFeatureIcpBooleanData,
            feature=boolean.get_variant().get_feature().index(),
            symbol=boolean.get_variant().get_symbol(),
        )
    )[0]
    assert concrete_boolean == boolean.get_variant()
    assert repository.insert(record(icp.dl.FeatureIcpBooleanData, variant=concrete_boolean.index()))[0] == boolean
    assert numerical.get_variant().get_feature() == numerical.get_variant().get_feature()
    assert numerical.get_variant().get_feature() == numerical.get_variant().get_feature()
    concrete_numerical = repository.insert(
        record(
            icp.dl.ConcreteFeatureIcpNumericalData,
            feature=numerical.get_variant().get_feature().index(),
            symbol=numerical.get_variant().get_symbol(),
        )
    )[0]
    assert concrete_numerical == numerical.get_variant()
    assert repository.insert(record(icp.dl.FeatureIcpNumericalData, variant=concrete_numerical.index()))[0] == numerical

    b, n = boolean.index(), numerical.index()
    _check_observation(
        lambda: repository.insert(record(icp.dl.ConcreteConditionIcpBooleanPositiveData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(icp.dl.ConcreteConditionIcpBooleanNegativeData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(icp.dl.ConcreteConditionIcpNumericalEqualZeroData, feature=n)), numerical
    )
    _check_observation(
        lambda: repository.insert(record(icp.dl.ConcreteConditionIcpNumericalGreaterZeroData, feature=n)), numerical
    )
    _check_observation(
        lambda: repository.insert(record(icp.dl.ConcreteEffectIcpBooleanPositiveData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(icp.dl.ConcreteEffectIcpBooleanNegativeData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(icp.dl.ConcreteEffectIcpBooleanUnchangedData, feature=b)), boolean
    )
    _check_observation(
        lambda: repository.insert(record(icp.dl.ConcreteEffectIcpNumericalIncreasesData, feature=n)), numerical
    )
    _check_observation(
        lambda: repository.insert(record(icp.dl.ConcreteEffectIcpNumericalDecreasesData, feature=n)), numerical
    )
    _check_observation(
        lambda: repository.insert(record(icp.dl.ConcreteEffectIcpNumericalUnchangedData, feature=n)), numerical
    )


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
    module_data.symbol = parsed_module.get_symbol().index()
    module_data.role_features = [role.index()]
    module_data.entry_memory_state = parsed_module.get_entry_memory_state().index()
    module_data.memory_states = [state.index() for state in parsed_module.get_memory_states()]
    module = repository.insert(module_data)[0]
    assert module == parsed_module
    assert list(module.get_role_features()) == [role]

    program_data = ext.ProgramData()
    program_data.entry_mod = module.index()
    program_data.modules = [module.index()]
    program = repository.insert(program_data)[0]
    assert program.get_entry_mod() == module
    assert list(program.get_modules()) == [module]
    assert ext.dl.parse_program(str(program), gripper_planning_domain, repository) == program


def test_data_rich_comparison_is_bound() -> None:
    first = base.RuleBaseData()
    first.symbol = "a"
    first_copy = base.RuleBaseData()
    first_copy.symbol = "a"
    second = base.RuleBaseData()
    second.symbol = "b"
    second_copy = base.RuleBaseData()
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
