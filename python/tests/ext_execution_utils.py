# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""Collect callback results for tests that inspect every immediate outcome."""

from collections.abc import Callable
from typing import TypeAlias

from pytyr.planning import ground, lifted

from pyrunir.datasets import GroundTaskSearchContext
from pyrunir.kr import GroundTaskContext, LiftedTaskContext
from pyrunir.kr.dl import semantics
from pyrunir.kr.ps import ext

TaskContext: TypeAlias = GroundTaskContext | LiftedTaskContext
Expander: TypeAlias = ext.GroundSuccessorExpander | ext.LiftedSuccessorExpander
ProgramState: TypeAlias = ext.GroundProgramState | ext.LiftedProgramState
EvaluationEnvironment: TypeAlias = ext.GroundEvaluationEnvironment | ext.LiftedEvaluationEnvironment
ExecutionStep: TypeAlias = ext.GroundProgramExecutionStep | ext.LiftedProgramExecutionStep
Outcome: TypeAlias = ExecutionStep | ext.ConceptChoice | ext.RoleChoice
PlanningState: TypeAlias = ground.State | lifted.State
DlContext: TypeAlias = semantics.GroundStateEvaluationContext | semantics.LiftedStateEvaluationContext


def initial_node(task_context: TaskContext) -> ground.Node | lifted.Node:
    """Return the planning search node of the task's initial state."""
    search = task_context.search_context
    if isinstance(search, GroundTaskSearchContext):
        return search.successor_generator.get_initial_node(search.state_repository, search.axiom_evaluator)
    return search.successor_generator.get_initial_node(search.state_repository, search.axiom_evaluator)


def make_expander(task_context: TaskContext, program: ext.Program) -> Expander:
    """Return the successor expander of the task context's kind."""
    if isinstance(task_context, GroundTaskContext):
        return ext.GroundSuccessorExpander(task_context, program)
    return ext.LiftedSuccessorExpander(task_context, program)


def make_environment(task_context: TaskContext, program: ext.Program) -> EvaluationEnvironment:
    """Return the evaluation environment of the task context's kind."""
    if isinstance(task_context, GroundTaskContext):
        return ext.GroundEvaluationEnvironment(task_context, program)
    return ext.LiftedEvaluationEnvironment(task_context, program)


def initial_program_state(expander: Expander, state: PlanningState) -> ProgramState:
    """Return the program state that starts `expander`'s program in the planning `state` of the same kind."""
    if isinstance(expander, ext.GroundSuccessorExpander):
        assert isinstance(state, ground.State)
        return expander.initial_state(state)
    assert isinstance(state, lifted.State)
    return expander.initial_state(state)


def make_dl_context(environment: EvaluationEnvironment, state: ProgramState) -> DlContext:
    """Return the description-logic evaluation context of `state` in `environment`."""
    if isinstance(environment, ext.GroundEvaluationEnvironment):
        assert isinstance(state, ext.GroundProgramState)
        return environment.make_dl_context(state)
    assert isinstance(state, ext.LiftedProgramState)
    return environment.make_dl_context(state)


def for_each_successor(
    expander: Expander,
    state: ProgramState,
    statistics: ext.ProgramSearchStatistics,
    emit: Callable[[Outcome], bool],
) -> bool:
    """Emit every immediate outcome of `state`, never stopping early."""
    if isinstance(expander, ext.GroundSuccessorExpander):
        assert isinstance(state, ext.GroundProgramState)
        return expander.for_each_successor(state, statistics, emit, lambda: False)
    assert isinstance(state, ext.LiftedProgramState)
    return expander.for_each_successor(state, statistics, emit, lambda: False)


def apply_choice(
    expander: Expander,
    state: ProgramState,
    choice: ext.ConceptChoice | ext.RoleChoice,
    statistics: ext.ProgramSearchStatistics,
) -> ExecutionStep:
    """Apply the current binding of `choice` to `state`."""
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

    assert for_each_successor(expander, state, statistics, emit)
    steps: list[ExecutionStep] = []
    for outcome in outcomes:
        if isinstance(outcome, (ext.ConceptChoice, ext.RoleChoice)):
            if outcome.exhausted():
                steps.append(apply_choice(expander, state, outcome, statistics))
            while not outcome.exhausted():
                steps.append(apply_choice(expander, state, outcome, statistics))
                outcome.advance()
        else:
            steps.append(outcome)
    return steps
