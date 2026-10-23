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

from .results import (
    Alternative,
    BindingRef,
    Budget,
    BudgetExhausted,
    ChoicePoint,
    ChooseRule,
    ChooseSummary,
    Contrast,
    Failure,
    Feature,
    LoadTrial,
    ModuleTermination,
    Plan,
    Step,
    Separation,
    Task,
    TaskRun,
    TaskValidation,
    Termination,
    Validation,
    Vertex,
    VertexRef,
)
from .session import Session

__all__ = [
    "Alternative",
    "BindingRef",
    "Budget",
    "BudgetExhausted",
    "ChoicePoint",
    "ChooseRule",
    "ChooseSummary",
    "Contrast",
    "Failure",
    "Feature",
    "LoadTrial",
    "ModuleTermination",
    "Plan",
    "Step",
    "Separation",
    "Session",
    "Task",
    "TaskRun",
    "TaskValidation",
    "Termination",
    "Validation",
    "Vertex",
    "VertexRef",
]
