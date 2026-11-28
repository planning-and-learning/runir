# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""The pytyr module of a search context's task kind, whose functions give its nodes and successors."""

from types import ModuleType

from pytyr.planning import ground, lifted

from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext


def planning_module(search: GroundTaskSearchContext | LiftedTaskSearchContext) -> ModuleType:
    """Return pytyr.planning.ground for a ground search context, pytyr.planning.lifted for a lifted one."""
    return ground if isinstance(search, GroundTaskSearchContext) else lifted
