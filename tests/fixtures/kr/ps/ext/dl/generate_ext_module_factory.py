"""Regenerate expected module-factory metadata."""

import json
from pathlib import Path
from typing import TypedDict, cast

from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pyrunir.kr.dl import ext as dl_ext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext import dl
from pytyr.formalism.planning import Parser


SUITE = Path(__file__).with_name("module_factory.json")


class Module(TypedDict):
    name: str
    concept_features: int
    role_features: int
    boolean_features: int
    numerical_features: int
    transitions: int
    syntactic_complexity: int


class Program(TypedDict):
    entry: str
    modules: list[str]
    syntactic_complexity: int


class Suite(TypedDict):
    domain_file: str
    modules: list[Module]
    program: Program


def main() -> None:
    suite = cast(Suite, json.loads(SUITE.read_text(encoding="utf-8")))
    planning_domain = Parser(Path(data_root()) / suite["domain_file"], ParserOptions()).get_domain()
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(planning_domain)
    repository = ext.RepositoryFactory().create(dl_repository)
    modules = dl.ModuleFactory.create_bonet_et_al_icaps2024_modules(planning_domain, repository)
    suite["modules"] = [
        {
            "name": module.get_name(),
            "concept_features": len(module.get_concept_features()),
            "role_features": len(module.get_role_features()),
            "boolean_features": len(module.get_boolean_features()),
            "numerical_features": len(module.get_numerical_features()),
            "transitions": len(module.get_memory_transitions()),
            "syntactic_complexity": module.syntactic_complexity(),
        }
        for module in modules
    ]
    program = dl.ModuleFactory.create_bonet_et_al_icaps2024_program(planning_domain, repository)
    suite["program"] = {
        "entry": program.get_entry_module().get_name(),
        "modules": [module.get_name() for module in program.get_modules()],
        "syntactic_complexity": program.syntactic_complexity(),
    }
    SUITE.write_text(json.dumps(suite, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
