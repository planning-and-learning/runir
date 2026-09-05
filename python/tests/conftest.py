from pypddl_datasets import data_root
from pathlib import Path

import pytest

from pyrunir.datasets import GroundTaskSearchContext
from pyyggdrasil.execution import ExecutionContext
from pypddl.formalism import ParserOptions
from pytyr.formalism.planning import Parser
from pytyr.planning.lifted import GroundTaskInstantiationOptions, Task


@pytest.fixture
def gripper_data_dir() -> Path:
    return data_root() / "classical" / "tests" / "gripper"


@pytest.fixture
def gripper_parser(gripper_data_dir: Path) -> Parser:
    return Parser(gripper_data_dir / "domain.pddl", ParserOptions())


@pytest.fixture
def gripper_planning_domain(gripper_parser: Parser):
    return gripper_parser.get_domain()


@pytest.fixture
def ground_gripper_search_context(gripper_data_dir: Path, gripper_parser: Parser):
    planning_task = gripper_parser.parse_task(gripper_data_dir / "test-1.pddl", ParserOptions())

    execution_context = ExecutionContext(1)
    lifted_task = Task(planning_task)
    ground_task = lifted_task.instantiate_ground_task(execution_context, GroundTaskInstantiationOptions()).task
    return GroundTaskSearchContext(ground_task, execution_context)
