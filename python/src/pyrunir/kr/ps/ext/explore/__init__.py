# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""Explore how an Ext module program behaves on lifted tasks, centred on its Choose rules.

A `Session` holds a domain, a program and named tasks. Searches run lazily per task with
`StateMemorization.ALL`, so the full explored graph is available: which bindings a Choose
tried, which failed, and how large the failed subtrees were. Untried bindings can be
evaluated with a budgeted search that continues from the binding's target configuration.

Names (rules, tasks, feature expressions) are accepted only at the boundary (`Session.rule`,
`Session.task`, `Session.feature`); every other call takes and returns handles.

Feature values follow the program's semantics: Boolean and numerical expressions are evaluated
in the configuration after binding, like `:order`; a concept expression is evaluated at the
Choose source, where candidates are computed, and its value for a binding is membership.
"""

from pyrunir.kr.ps.ext.explore.results import (
    Alternative as Alternative,
    BindingRef as BindingRef,
    Budget as Budget,
    BudgetExhaustedError as BudgetExhaustedError,
    ChoicePoint as ChoicePoint,
    ChooseRule as ChooseRule,
    ChooseSummary as ChooseSummary,
    Contrast as Contrast,
    DeadEnd as DeadEnd,
    Failure as Failure,
    Feature as Feature,
    LoadTrial as LoadTrial,
    ModuleTermination as ModuleTermination,
    Plan as Plan,
    Separation as Separation,
    SessionOptions as SessionOptions,
    Step as Step,
    Task as Task,
    TaskRun as TaskRun,
    TaskValidation as TaskValidation,
    Termination as Termination,
    Validation as Validation,
    Vertex as Vertex,
    VertexRef as VertexRef,
)
from pyrunir.kr.ps.ext.explore.session import Session as Session

__all__ = [
    "Alternative",
    "BindingRef",
    "Budget",
    "BudgetExhaustedError",
    "ChoicePoint",
    "ChooseRule",
    "ChooseSummary",
    "Contrast",
    "DeadEnd",
    "Failure",
    "Feature",
    "LoadTrial",
    "ModuleTermination",
    "Plan",
    "Separation",
    "Session",
    "SessionOptions",
    "Step",
    "Task",
    "TaskRun",
    "TaskValidation",
    "Termination",
    "Validation",
    "Vertex",
    "VertexRef",
]
