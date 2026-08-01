"""Regenerate base structural-termination status fields."""

import json
from pathlib import Path
from typing import TypedDict, cast

from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pyrunir.kr.dl.base.semantics import ConstructorRepositoryFactory as BaseDlRepositoryFactory
from pyrunir.kr.ps.base import RepositoryFactory as BaseRepositoryFactory
from pyrunir.kr.ps.base.dl import parse_sketch, structural_termination as base_structural_termination
from pytyr.formalism.planning import Parser


ROOT = Path(__file__).resolve().parents[4]
SUITE = Path(__file__).with_name("structural_termination.json")


class Case(TypedDict):
    name: str
    file: str
    terminating: bool


class Suite(TypedDict):
    base: list[Case]


def main() -> None:
    suite = cast(Suite, json.loads(SUITE.read_text(encoding="utf-8")))
    planning_domain = Parser(Path(data_root()) / "classical/tests/gripper/domain.pddl", ParserOptions()).get_domain()

    base_dl_repository = BaseDlRepositoryFactory().create(planning_domain)
    base_repository = BaseRepositoryFactory().create(base_dl_repository)
    for case in suite["base"]:
        description = (ROOT / "tests/fixtures" / case["file"]).read_text(encoding="utf-8")
        case["terminating"] = base_structural_termination(parse_sketch(description, planning_domain, base_repository)).is_terminating()

    SUITE.write_text(json.dumps(suite, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
