from pathlib import Path

import pyrunir
import pytest


def test_python_bindings_expose_submodules():
    assert pyrunir.graphs.DynamicGraph is not None
    assert pyrunir.kr.dl.base.grammar.ConstructorRepository is not None


def test_native_prefix_points_to_installed_cmake_package():
    if not hasattr(pyrunir, "native_prefix"):
        pytest.skip("native_prefix is only available through the installed pyrunir package wrapper")

    prefix = pyrunir.native_prefix()

    assert (prefix / "include" / "runir" / "runir.hpp").exists()
    assert (prefix / "lib" / "cmake" / "runir" / "runirConfig.cmake").exists()


def test_feature_complexity_methods_are_in_public_stubs():
    expected = {
        "kr/ps/base/dl": ("BooleanFeature", "NumericalFeature", "ConcreteBooleanFeature", "ConcreteNumericalFeature"),
        "kr/ps/base": ("Sketch",),
        "kr/ps/ext/dl": (
            "ConceptFeature",
            "RoleFeature",
            "BooleanFeature",
            "NumericalFeature",
            "ConcreteConceptFeature",
            "ConcreteRoleFeature",
            "ConcreteBooleanFeature",
            "ConcreteNumericalFeature",
        ),
        "kr/ps/ext": ("Module", "ModuleProgram"),
        "kr/uns/dl": ("Feature",),
        "kr/uns": ("Feature", "Classifier"),
    }
    package = Path(pyrunir.__file__).parent

    for relative, classes in expected.items():
        stub = package / relative / "__init__.pyi"
        lines = stub.read_text(encoding="utf-8").splitlines()
        for class_name in classes:
            start = lines.index(f"class {class_name}:") + 1
            body = []
            for line in lines[start:]:
                if line and not line.startswith(" "):
                    break
                body.append(line)
            assert "    def syntactic_complexity(self) -> int: ..." in body


def test_structural_termination_incomplete_result_is_in_public_stub():
    stub = Path(pyrunir.__file__).parent / "kr/ps/base/dl/__init__.pyi"

    assert "    def incomplete_result(self) -> IncompleteStructuralTerminationResult | None: ..." in stub.read_text(encoding="utf-8")
