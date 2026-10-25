# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""Collect callback results for tests that inspect every immediate outcome."""

from typing import TypeAlias

from pytyr.planning import ground, lifted

from pyrunir.datasets import GroundTaskSearchContext
from pyrunir.kr import GroundTaskContext, LiftedTaskContext
from pyrunir.kr.ps import ext

TaskContext: TypeAlias = GroundTaskContext | LiftedTaskContext
Expander: TypeAlias = ext.GroundSuccessorExpander | ext.LiftedSuccessorExpander
ProgramState: TypeAlias = ext.GroundProgramState | ext.LiftedProgramState
EvaluationEnvironment: TypeAlias = ext.GroundEvaluationEnvironment | ext.LiftedEvaluationEnvironment
ExecutionStep: TypeAlias = ext.GroundProgramExecutionStep | ext.LiftedProgramExecutionStep
Outcome: TypeAlias = ExecutionStep | ext.ConceptChoice | ext.RoleChoice


def initial_node(task_context: TaskContext) -> ground.Node | lifted.Node:
    """Return the planning search node of the task's initial state."""
    search = task_context.search_context
    if isinstance(search, GroundTaskSearchContext):
        return search.successor_generator.get_initial_node(search.state_repository, search.axiom_evaluator)
    return search.successor_generator.get_initial_node(search.state_repository, search.axiom_evaluator)


def _apply_choice(
    expander: Expander,
    state: ProgramState,
    choice: ext.ConceptChoice | ext.RoleChoice,
    statistics: ext.ProgramSearchStatistics,
) -> ExecutionStep:
    if isinstance(expander, ext.GroundSuccessorExpander):
        assert isinstance(state, ext.GroundProgramState)
        return expander.apply_choice(state, choice, statistics)
    assert isinstance(state, ext.LiftedProgramState)
    return expander.apply_choice(state, choice, statistics)


def collect_steps(
    expander: Expander, state: ProgramState, statistics: ext.ProgramSearchStatistics | None = None
) -> list[ExecutionStep]:
    """Expand ``state`` and return every immediate step, applying each binding of every emitted choice."""
    if statistics is None:
        statistics = ext.ProgramSearchStatistics()
    outcomes: list[Outcome] = []

    def emit(outcome: Outcome) -> bool:
        outcomes.append(outcome)
        return True

    assert expander.for_each_successor(state, statistics, emit, lambda: False)
    steps: list[ExecutionStep] = []
    for outcome in outcomes:
        if isinstance(outcome, (ext.ConceptChoice, ext.RoleChoice)):
            if outcome.exhausted():
                steps.append(_apply_choice(expander, state, outcome, statistics))
            while not outcome.exhausted():
                steps.append(_apply_choice(expander, state, outcome, statistics))
                outcome.advance()
        else:
            steps.append(outcome)
    return steps
