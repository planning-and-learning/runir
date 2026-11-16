# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import json
from pathlib import Path
from typing import TypeVar, cast

RecordT = TypeVar("RecordT")

FIXTURE_ROOT = Path(__file__).resolve().parents[2] / "tests" / "support" / "data"


def read_fixture(relative: str) -> str:
    return (FIXTURE_ROOT / relative).read_text(encoding="utf-8")


def load_fixture(relative: str) -> dict[str, object]:
    value: object = json.loads(read_fixture(relative))
    assert isinstance(value, dict)
    return cast("dict[str, object]", value)


def record(data_type: type[RecordT], **fields: object) -> RecordT:
    """A record built as Python builds one: its empty __init__, then each field assigned.

    A view, or a list of views, is stored as its index.
    """
    data = data_type()
    for name, value in fields.items():
        setattr(data, name, value)
    return data
