import pytemplate_feature


def test_python_bindings_expose_core_api():
    summary = pytemplate_feature.describe("planner", 7)

    assert pytemplate_feature.add(20, 22) == 42
    assert pytemplate_feature.format_summary(summary) == "planner: 7"
    assert summary.name == "planner"
    assert summary.score == 7


def test_native_prefix_points_to_installed_cmake_package():
    prefix = pytemplate_feature.native_prefix()

    assert (prefix / "include" / "template_feature" / "template_feature.hpp").exists()
    assert (prefix / "lib" / "cmake" / "template_feature" / "template_featureConfig.cmake").exists()

