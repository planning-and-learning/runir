"""Regenerate equivalence-graph counts while preserving authored inputs."""

import json
from pathlib import Path
from typing import TypedDict, cast

from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pyrunir.datasets import (
    EquivalenceGraphGenerationOptions,
    EquivalencePolicyMode,
    GroundTaskSearchContext,
    generate_ground_equivalence_graph,
)
from pyyggdrasil.execution import ExecutionContext
from pytyr.formalism.planning import Parser
from pytyr.planning.lifted import GroundTaskInstantiationOptions, Task


SUITE = Path(__file__).with_name("equivalence_graph.json")
POLICIES = {
    "identity": EquivalencePolicyMode.IDENTITY,
    "gi": EquivalencePolicyMode.GI,
}


class Case(TypedDict):
    name: str
    domain_file: str
    task_files: list[str]
    equivalence_policy: str
    expected_num_vertices: int
    expected_num_edges: int


class Suite(TypedDict):
    tests: list[Case]


def make_contexts(case: Case) -> list[GroundTaskSearchContext]:
    root = Path(data_root())
    parser = Parser(root / str(case["domain_file"]), ParserOptions())
    execution_context = ExecutionContext(1)
    contexts: list[GroundTaskSearchContext] = []
    for task_file in case["task_files"]:
        planning_task = parser.parse_task(root / task_file, ParserOptions())
        task = Task(planning_task).instantiate_ground_task(execution_context, GroundTaskInstantiationOptions()).task
        contexts.append(GroundTaskSearchContext(task, execution_context))
    return contexts


def main() -> None:
    suite = cast(Suite, json.loads(SUITE.read_text(encoding="utf-8")))
    for case in suite["tests"]:
        options = EquivalenceGraphGenerationOptions()
        options.policy_mode = POLICIES[case["equivalence_policy"]]
        result = generate_ground_equivalence_graph(make_contexts(case), options)
        graph = result.graph.get_forward_graph()
        case["expected_num_vertices"] = graph.get_num_vertices()
        case["expected_num_edges"] = graph.get_num_edges()
    SUITE.write_text(json.dumps(suite, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
