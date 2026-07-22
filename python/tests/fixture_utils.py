import json
from pathlib import Path
from typing import cast


FIXTURE_ROOT = Path(__file__).resolve().parents[2] / "tests" / "fixtures"


def fixture_path(relative: str) -> Path:
    return FIXTURE_ROOT / relative


def read_fixture(relative: str) -> str:
    return fixture_path(relative).read_text(encoding="utf-8")


def load_fixture(relative: str) -> dict[str, object]:
    value: object = json.loads(read_fixture(relative))
    assert isinstance(value, dict)
    return cast(dict[str, object], value)
