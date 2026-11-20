# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""Reading relations by decoded values, and renaming an interned relation's columns."""

from collections.abc import Sequence
from typing import Any

from pyyggdrasil import formalism
from pyyggdrasil.formalism import ObjectIndex


# `row` is a row of a builder (RowBuilderView) or of an interned relation (RowSchema); the bindings give them no common
# base.
def values(row: Any) -> tuple[object, ...]:  # noqa: ANN401
    """The decoded values of `row`, an object column as its index (an int)."""
    return tuple(_plain(row.get(i)) for i in range(len(row.get_values())))


def _plain(value: object) -> object:
    """An object index (ObjectIndex) as its int, any other decoded value as it is."""
    return int(value) if isinstance(value, ObjectIndex) else value


def rename(
    repository: formalism.RelationRepository, relation: formalism.RelationSchema, new: Sequence[formalism.ColumnIndex]
) -> formalism.RelationSchema:
    """`relation` (of `repository`) under new labels: the same pool and rows."""
    columns = relation.columns()
    if len(new) != len(columns):
        message = "Columns: renaming must preserve arity."
        raise ValueError(message)
    layout = [formalism.ColumnLayoutData(label, column.type) for label, column in zip(new, columns, strict=True)]
    renamed = repository.insert(formalism.ColumnsData(layout))[0]
    return repository.insert(
        formalism.RelationSchemaData(
            relation.get_pool().index(), renamed.index(), [each.index() for each in relation.get_rows()]
        )
    )[0]
