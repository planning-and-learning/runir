from pypddl_datasets import data_root
from pathlib import Path

import pytest

from pyrunir.datasets import GroundTaskSearchContext
from pyyggdrasil.execution import ExecutionContext
from pypddl.formalism import ParserOptions
from pytyr.formalism.planning import Parser
from pytyr.planning.lifted import GroundTaskInstantiationOptions, Task


@pytest.fixture
def repository_root() -> Path:
    return Path(__file__).resolve().parents[2]


@pytest.fixture
def gripper_data_dir() -> Path:
    return data_root() / "classical" / "tests" / "gripper"


@pytest.fixture
def gripper_planning_domain(gripper_data_dir: Path):
    parser = Parser(gripper_data_dir / "domain.pddl", ParserOptions())
    return parser.get_domain()


@pytest.fixture
def ground_gripper_search_context(gripper_data_dir: Path):
    parser_options = ParserOptions()
    parser = Parser(gripper_data_dir / "domain.pddl", parser_options)
    planning_task = parser.parse_task(gripper_data_dir / "test-1.pddl", parser_options)

    execution_context = ExecutionContext(1)
    lifted_task = Task(planning_task)
    ground_task = lifted_task.instantiate_ground_task(execution_context, GroundTaskInstantiationOptions()).task
    return GroundTaskSearchContext(ground_task, execution_context)
