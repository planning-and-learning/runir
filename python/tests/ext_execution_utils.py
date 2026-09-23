"""Collect callback results for tests that inspect every immediate outcome."""

from pyrunir.kr.ps import ext


def initial_node(task_context):
    search = task_context.search_context
    return search.successor_generator.get_initial_node(search.state_repository, search.axiom_evaluator)


def collect_steps(expander, state, statistics=None):
    if statistics is None:
        statistics = ext.ProgramSearchStatistics()
    outcomes = []

    def emit(outcome):
        outcomes.append(outcome)
        return True

    assert expander.for_each_successor(state, statistics, emit, lambda: False)
    steps = []
    for outcome in outcomes:
        if isinstance(outcome, (ext.ConceptChoice, ext.RoleChoice)):
            if outcome.exhausted():
                steps.append(expander.apply_choice(state, outcome, statistics))
            while not outcome.exhausted():
                steps.append(expander.apply_choice(state, outcome, statistics))
                outcome.advance()
        else:
            steps.append(outcome)
    return steps
