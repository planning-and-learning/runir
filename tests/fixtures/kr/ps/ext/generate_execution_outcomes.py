"""Regenerate execution outcome fields while preserving program inputs."""

import json
from pathlib import Path
from typing import TypedDict, cast

from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pyrunir.datasets import GroundTaskSearchContext
from pyrunir.kr import GroundTaskContext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext import dl
from pyyggdrasil.execution import ExecutionContext
from pytyr.formalism.planning import Parser
from pytyr.planning.lifted import GroundTaskInstantiationOptions, Task


ROOT = Path(__file__).resolve().parents[5]
SUITE = Path(__file__).with_name("execution.json")


class Case(TypedDict):
    name: str
    program_file: str
    universal: bool
    status: str
    num_vertices: int
    num_edges: int
    num_deadends: int
    has_open_states: bool
    cycle_length: int


class Suite(TypedDict):
    cases: list[Case]


def main() -> None:
    suite = cast(Suite, json.loads(SUITE.read_text(encoding="utf-8")))
    benchmark = Path(data_root()) / "classical/profiling/blocksworld-large-simple"
    parser = Parser(benchmark / "domain.pddl", ParserOptions())
    planning_task = parser.parse_task(benchmark / "p-100-2.pddl", ParserOptions())
    planning_domain = parser.get_domain()
    execution_context = ExecutionContext(1)
    task = Task(planning_task).instantiate_ground_task(execution_context, GroundTaskInstantiationOptions()).task
    task_context = GroundTaskContext(GroundTaskSearchContext(task, execution_context))

    for case in suite["cases"]:
        description = (ROOT / "tests/fixtures" / case["program_file"]).read_text(encoding="utf-8")
        program = dl.parse_module_program(description, planning_domain, task_context.ext_repository)
        options = ext.GroundModuleProgramSearchOptions()
        options.universal = case["universal"]
        result = ext.find_ground_solution(task_context, program, options)
        case["status"] = {
            ext.ModuleProgramProofStatus.SUCCESS: "success",
            ext.ModuleProgramProofStatus.FAILURE: "failure",
            ext.ModuleProgramProofStatus.OUT_OF_TIME: "out_of_time",
            ext.ModuleProgramProofStatus.OUT_OF_STATES: "out_of_states",
        }[result.status]
        case["num_vertices"] = result.graph.get_num_vertices()
        case["num_edges"] = result.graph.get_num_edges()
        case["num_deadends"] = len(result.deadend_states)
        case["has_open_states"] = bool(result.open_states)
        case["cycle_length"] = len(result.cycle)

    SUITE.write_text(json.dumps(suite, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
