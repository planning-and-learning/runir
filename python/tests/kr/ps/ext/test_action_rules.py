import gc

import pytest
from ext_execution_utils import collect_steps, initial_node
from fixture_utils import FIXTURE_ROOT
from pypddl.formalism import ParserOptions
from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext
from pyrunir.kr import ArityMismatchError, DomainContext, GroundTaskContext, LiftedTaskContext, UndefinedSymbolError
from pyrunir.kr.dl.base import semantics as base_semantics
from pyrunir.kr.dl.ext import semantics
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext import dl
from pyrunir.serialization import register_table, serialize, table
from pytyr.formalism.planning import Object, Parser
from pytyr.planning import lifted
from pyyggdrasil.execution import ExecutionContext
from pyyggdrasil.serialization import Dictionaries


QUERY = '(q_join (q_atomic_state "at" (from)) (q_atomic_state "edge" (from to)))'


def _program(query=QUERY, effects="(unchanged count)"):
    return f"""(:program (:entry root)
      (:module (:symbol root) (:arguments) (:registers)
        (:entry m0) (:memory m0)
        (:features
          (:query (:symbol selected) (:expression {query}))
          (:query (:symbol edges) (:expression (q_atomic_state "edge" (from to))))
          (:query (:symbol reversed) (:expression (q_project (to from) {QUERY})))
          (:query (:symbol truth) (:expression (q_project () (q_atomic_state "at" (from)))))
          (:query (:symbol empty) (:expression
            (q_project () (q_difference (q_atomic_state "at" (from)) (q_atomic_state "at" (from))))))
          (:numerical (:symbol count) (:expression (n_count (c_atomic_state "at")))))
        (:rules (:rule (:symbol move-selected) (:expression
          (:source-memory m0) (:target-memory m0)
          (:action (:conditions) (:action "move") (:query selected) (:effects {effects})))))))"""


def _context(kind):
    directory = FIXTURE_ROOT / "kr/ps/ext/choose"
    parser = Parser(directory / "domain.pddl", ParserOptions())
    task = lifted.Task(parser.parse_task(directory / "task.pddl", ParserOptions()))
    execution = ExecutionContext(1)
    domain = DomainContext(parser.get_domain())
    if kind == "ground":
        context = GroundTaskContext(domain, GroundTaskSearchContext(task.instantiate_ground_task(execution).task, execution))
    else:
        context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
    return context, parser.get_domain()


def _runtime(kind, source=None):
    task_context, domain = _context(kind)
    program = dl.parse_program(source or _program(), domain, task_context.domain_context.ext_repository)
    prefix = kind.title()
    expander = getattr(ext, f"{prefix}SuccessorExpander")(task_context, program)
    state = expander.initial_state(initial_node(task_context).get_state())
    environment = getattr(ext, f"{prefix}EvaluationEnvironment")(task_context, program)
    return task_context, program, expander, state, environment


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_expander_rejects_foreign_states_programs_and_candidates(kind):
    task_context, domain = _context(kind)
    repository = task_context.domain_context.ext_repository
    program = dl.parse_program(_program(), domain, repository)
    expander_type = getattr(ext, f"{kind.title()}SuccessorExpander")
    expander = expander_type(task_context, program)
    planning_state = initial_node(task_context).get_state()
    state = expander.initial_state(planning_state)
    step = collect_steps(expander, state)[0]
    successor = step.planning_successor.unpack()
    foreign_context, foreign_program, foreign_expander, foreign_state, foreign_environment = _runtime(kind)
    other_program = dl.parse_program(_program(effects=""), domain, repository)
    other_expander = expander_type(task_context, other_program)
    other_state = other_expander.initial_state(planning_state)

    with pytest.raises(ValueError, match="domain context repository"):
        expander_type(task_context, foreign_program)
    with pytest.raises(ValueError, match="selected task's state repository"):
        expander.initial_state(foreign_state.state)
    for wrong_state, message in ((foreign_state, "selected task"), (other_state, "selected program")):
        statistics = ext.ProgramSearchStatistics()
        with pytest.raises(ValueError, match=message):
            expander.for_each_successor(wrong_state, statistics, lambda _: True, lambda: False)
        assert statistics.num_generated == statistics.num_expanded == 0
        with pytest.raises(ValueError, match=message):
            expander.matching_rule(wrong_state, successor)
        with pytest.raises(ValueError, match=message):
            expander.apply(wrong_state, step.rule, successor)

    foreign_successor = collect_steps(foreign_expander, foreign_state)[0].planning_successor.unpack()
    with pytest.raises(ValueError, match="selected task's state repository"):
        expander.matching_rule(state, foreign_successor)
    with pytest.raises(ValueError, match="selected task's state repository"):
        expander.apply(state, step.rule, foreign_successor)


def test_action_and_query_feature_bindings_round_trip_and_serialize():
    task_context, domain = _context("lifted")
    repository = task_context.domain_context.ext_repository
    program = dl.parse_program(_program(), domain, repository)
    module = program.get_entry_module()
    feature = module.get_query_features()[0]
    variant = module.get_memory_transitions()[0][0]
    rule = variant.get_variant()
    assert isinstance(feature, dl.QueryFeature)
    assert isinstance(feature.get_variant(), dl.ConcreteQueryFeature)
    assert isinstance(rule, ext.ActionRule)
    assert rule.get_query_feature() == feature
    assert rule.get_action_name() == "move"
    assert [column.get_name() for column in feature.get_expression().get_columns()] == ["from", "to"]
    assert feature.syntactic_complexity() > 0
    assert str(dl.parse_program(str(program), domain, repository)) == str(program)

    concrete = dl.ConcreteQueryFeatureData()
    concrete.symbol = feature.get_symbol()
    concrete.feature = feature.get_expression().get_index()
    assert repository.insert(concrete)[0] == feature.get_variant()
    wrapped = dl.QueryFeatureData()
    wrapped.variant = feature.get_variant().get_index()
    assert repository.insert(wrapped)[0] == feature
    data = ext.ActionRuleData()
    data.source = rule.get_source().get_index()
    data.target = rule.get_target().get_index()
    data.conditions = [condition.get_index() for condition in rule.get_conditions()]
    data.effects = [effect.get_index() for effect in rule.get_effects()]
    data.action_name = rule.get_action_name()
    data.query_feature = feature.get_index()
    assert repository.insert(data)[0] == rule
    module_data = ext.ModuleData()
    module_data.query_features = [feature.get_index()]
    assert module_data.query_features == [feature.get_index()]

    dictionaries = Dictionaries()
    register_table(dictionaries, ext.RuleVariant, "rules", "r")
    register_table(dictionaries, ext.ActionRule, "actions", "a", fields=("action_name", "query_feature"))
    register_table(dictionaries, dl.QueryFeature, "queries", "q", project=lambda value: {
        "symbol": value.get_symbol(), "expression": str(value.get_expression()),
    })
    assert serialize(dictionaries, variant) == "r0"
    assert table(dictionaries, ext.ActionRule) == [{"action_name": "move", "query_feature": "q0"}]
    assert table(dictionaries, dl.QueryFeature) == [{"symbol": "selected", "expression": QUERY}]


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_query_relation_preserves_rows_column_order_nullary_truth_and_lifetimes(kind):
    task_context, program, expander, state, environment = _runtime(kind)
    features = {feature.get_symbol(): feature for feature in program.get_entry_module().get_query_features()}

    dl_context = environment.make_dl_context(state)
    relation = ext.evaluate(features["selected"], dl_context)
    assert isinstance(relation, base_semantics.QueryDenotation)
    assert relation == features["selected"].get_expression().evaluate(dl_context)
    assert len(relation) == 2
    assert relation.arity() == 2
    assert len(relation.columns()) == 2
    assert relation == ext.evaluate(features["selected"], dl_context)
    assert len({relation, ext.evaluate(features["selected"], dl_context)}) == 1
    assert all(isinstance(row, base_semantics.QueryDenotationRow) and len(row) == 2 for row in relation)
    rows = {tuple(row) for row in relation}
    assert all(isinstance(value, Object) for row in rows for value in row)
    assert {tuple(value.get_name() for value in row) for row in rows} == {("start", "bad"), ("start", "good")}
    assert relation[-1][-1] == relation.at(len(relation) - 1)[relation.arity() - 1]
    with pytest.raises(IndexError):
        relation[len(relation)]
    with pytest.raises(IndexError):
        relation[0][relation.arity()]
    assert {tuple(row) for row in ext.evaluate(features["reversed"], dl_context)} == {
        (target, source) for source, target in rows
    }
    truth = ext.evaluate(features["truth"], dl_context)
    empty = ext.evaluate(features["empty"], dl_context)
    assert truth.arity() == empty.arity() == 0
    assert [tuple(row) for row in truth] == [()]
    assert empty.empty()
    assert {tuple(row) for row in ext.evaluate(features["selected"], dl_context)} == rows
    assert {tuple(row) for row in relation} == rows

    steps = collect_steps(expander, state)
    assert len(steps) == 2
    assert rows == {
        tuple(step.state_transition.action.get_objects())
        for step in steps
    }
    for step in steps:
        assert isinstance(step.rule.get_variant(), ext.ActionRule)
        successor = step.planning_successor.unpack()
        assert expander.matching_rule(state, successor) == step.rule
        assert expander.apply(state, step.rule, successor).target == step.target
    good_step = next(step for step in steps if step.state_transition.action.get_objects()[1].get_name() == "good")
    del relation, truth, empty
    state = good_step.target
    environment.reset_source()
    dl_context = environment.make_dl_context(state)
    relation = ext.evaluate(features["selected"], dl_context)
    assert len(relation) == 1
    new_rows = {tuple(row) for row in relation}
    row = relation.at(0)
    iterated_row = next(iter(relation))
    expected_row = tuple(row)
    del steps, good_step, successor, step, features, environment, dl_context, state, expander, program, task_context
    gc.collect()
    assert {tuple(value) for value in relation} == new_rows
    del relation
    gc.collect()
    assert tuple(row) == tuple(iterated_row) == expected_row


@pytest.mark.parametrize("kind", ["ground", "lifted"])
@pytest.mark.parametrize("access", ["object", "row_iterator", "query_iterator", "renamed"])
def test_query_objects_iterators_and_renames_retain_owners(kind, access):
    def retained_value():
        task_context, program, expander, state, environment = _runtime(kind)
        repository = task_context.dl_denotation_repository
        caches = semantics.DenotationCaches()
        storage = semantics.EvaluationStorage(repository)
        context = getattr(semantics, f"{kind.title()}StateEvaluationContext")(
            state.state, task_context.dl_builder, caches, repository, storage,
            state.module_state.arguments, state.module_state.registers,
        )
        feature = program.get_entry_module().get_query_features()[0]
        relation = feature.get_expression().evaluate(context)
        expected = tuple(object_.get_name() for object_ in relation[0])
        if access == "object":
            return relation[0][0], expected
        if access == "row_iterator":
            return iter(relation[0]), expected
        if access == "query_iterator":
            return iter(relation), expected
        columns = list(relation.columns())[::-1]
        renamed = repository.get_relation_repository().rename(relation, columns)
        assert isinstance(renamed, base_semantics.QueryDenotation)
        assert list(renamed.columns()) == columns
        return renamed, expected

    value, expected = retained_value()
    gc.collect()
    if access == "object":
        object_ = value
    elif access == "row_iterator":
        object_ = next(value)
    else:
        row = next(value) if access == "query_iterator" else value[0]
        assert tuple(object_.get_name() for object_ in row) == expected
        object_ = next(iter(row))
        del row
    del value
    gc.collect()
    assert isinstance(object_, Object)
    assert object_.get_name() == expected[0]


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_action_effect_contract_violation_is_an_exception(kind):
    task_context, program, expander, state, environment = _runtime(kind, _program(effects="(decreases count)"))
    with pytest.raises(RuntimeError, match="offered transition violates declared effects"):
        collect_steps(expander, state)
    options = getattr(ext, f"{kind.title()}ProgramSearchOptions")()
    with pytest.raises(RuntimeError, match="offered transition violates declared effects"):
        getattr(ext, f"find_{kind}_solution")(task_context, program, options)


@pytest.mark.parametrize("query", ['(q_atomic_state "at" (from))', '(q_project () (q_atomic_state "at" (from)))'])
def test_action_parser_rejects_incomplete_binding_schema(query):
    task_context, domain = _context("lifted")
    with pytest.raises(ArityMismatchError):
        dl.parse_program(_program(query=query), domain, task_context.domain_context.ext_repository)


def test_action_parser_rejects_undefined_query_feature():
    task_context, domain = _context("lifted")
    with pytest.raises(UndefinedSymbolError):
        dl.parse_program(_program().replace("(:query selected)", "(:query missing)"), domain, task_context.domain_context.ext_repository)


@pytest.mark.parametrize("preprocessing", [False, True])
def test_action_structural_counterexample_retains_query_rule(preprocessing):
    task_context, domain = _context("lifted")
    program = dl.parse_program(_program(), domain, task_context.domain_context.ext_repository)
    result = dl.structural_termination(program.get_entry_module(), use_incomplete_preprocessing=preprocessing)
    assert not result.is_terminating()
    graph = result.sieve_result.counterexample
    rule = result.sieve_result.surviving_rules[0]
    del result, program  # Rule views keep the existing caller-owned repository contract.
    gc.collect()
    assert isinstance(rule.get_variant(), ext.ActionRule)
    assert rule.get_variant().get_query_feature().get_symbol() == "selected"
    assert all(isinstance(graph.get_edge_property(index).get_variant(), ext.ActionRule) for index in graph.get_edge_indices())


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_action_rejects_visited_inapplicable_query_tuple(kind):
    task_context, program, expander, state, environment = _runtime(
        kind, _program(query='(q_atomic_state "edge" (from to))'),
    )
    with pytest.raises(RuntimeError, match="offered action is not applicable"):
        collect_steps(expander, state)


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_static_query_role_closure_query_composition(kind):
    closure = '(q_role (from to) (r_transitive_closure (r_project from to (q_atomic_state "edge" (from to)))))'
    task_context, program, expander, state, environment = _runtime(kind, _program(query=closure))
    features = {feature.get_symbol(): feature for feature in program.get_entry_module().get_query_features()}

    dl_context = environment.make_dl_context(state)
    edges = {tuple(row) for row in ext.evaluate(features["edges"], dl_context)}
    rows = {tuple(row) for row in ext.evaluate(features["selected"], dl_context)}
    assert len(edges) == 3
    assert len(rows) == 4
    assert rows == edges | {(source, target) for source, middle in edges for other, target in edges if middle == other}
    search = task_context.search_context
    node = initial_node(task_context)
    successor = search.successor_generator.get_labeled_successor_nodes(
        node, search.state_repository, search.axiom_evaluator,
    )[0]
    rule = expander.matching_rule(state, successor)
    assert rule is not None
    state = expander.apply(state, rule, successor).target
    environment.reset_source()
    dl_context = environment.make_dl_context(state)
    assert {tuple(row) for row in ext.evaluate(features["selected"], dl_context)} == rows


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_query_features_follow_module_arguments_and_registers_in_the_same_state(kind):
    source = """(:program (:entry root)
      (:module (:symbol root) (:arguments) (:registers)
        (:entry m0) (:memory m0 m1)
        (:features (:concept (:symbol candidates) (:expression (c_atomic_state "candidate"))))
        (:rules (:rule (:symbol enter) (:expression
          (:source-memory m0) (:target-memory m1)
          (:call (:conditions) (:callee child) (:arguments candidates))))))
      (:module (:symbol child) (:arguments (:concept X)) (:registers (:concept selected))
        (:entry m0) (:memory m0 m1)
        (:features
          (:concept (:symbol candidates) (:expression (c_argument X)))
          (:query (:symbol argument) (:expression (q_concept choice (c_argument X))))
          (:query (:symbol register) (:expression (q_concept choice (c_register selected))))
          (:query (:symbol intersection) (:expression
            (q_join (q_concept choice (c_argument X)) (q_concept choice (c_register selected))))))
        (:rules (:rule (:symbol bind) (:expression
          (:source-memory m0) (:target-memory m1)
          (:load (:conditions) (:concept candidates) (:register (:concept selected))))))))"""
    task_context, program, expander, initial, environment = _runtime(kind, source)
    child = collect_steps(expander, initial)[0].target
    choices = [step.target for step in collect_steps(expander, child)]
    assert len(choices) == 2
    assert choices[0].state == choices[1].state == initial.state
    features = {feature.get_symbol(): feature for feature in child.module_state.module.get_query_features()}
    expected = [((state.module_state.registers.concept_values[0],),) for state in choices]
    all_candidates = {row for selected in expected for row in selected}
    assert expected[0] != expected[1]

    snapshots = []
    for position in (0, 1, 0):
        state = choices[position]
        environment.reset_source()
        dl_context = environment.make_dl_context(state)
        assert {tuple(row) for row in ext.evaluate(features["argument"], dl_context)} == all_candidates
        selected = ext.evaluate(features["register"], dl_context)
        assert tuple(tuple(row) for row in selected) == expected[position]
        intersection = ext.evaluate(features["intersection"], dl_context)
        assert tuple(tuple(row) for row in intersection) == expected[position]
        snapshots.append(tuple(tuple(int(object_.get_index()) for object_ in row) for row in selected))
        del selected, intersection
    expected_indices = [tuple(tuple(int(object_.get_index()) for object_ in row) for row in rows) for rows in expected]
    assert snapshots == [expected_indices[0], expected_indices[1], expected_indices[0]]


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_action_query_preserves_correlated_parameter_tuples(kind, tmp_path):
    domain_file = tmp_path / "domain.pddl"
    task_file = tmp_path / "task.pddl"
    domain_file.write_text("""(define (domain correlated)
      (:requirements :strips :typing)
      (:types source target)
      (:predicates (allowed ?s - source ?t - target) (linked ?s - source ?t - target))
      (:action connect :parameters (?s - source ?t - target)
        :precondition (and) :effect (linked ?s ?t)))""")
    task_file.write_text("""(define (problem correlated-task) (:domain correlated)
      (:objects a b - source c d - target)
      (:init (allowed a c) (allowed b d))
      (:goal (and (linked a c) (linked a d) (linked b c) (linked b d))))""")
    parser = Parser(domain_file, ParserOptions())
    task = lifted.Task(parser.parse_task(task_file, ParserOptions()))
    execution = ExecutionContext(1)
    domain = DomainContext(parser.get_domain())
    if kind == "ground":
        task_context = GroundTaskContext(domain, GroundTaskSearchContext(task.instantiate_ground_task(execution).task, execution))
    else:
        task_context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
    program = dl.parse_program("""(:program (:entry root)
      (:module (:symbol root) (:arguments) (:registers)
        (:entry m0) (:memory m0 m1)
        (:features (:query (:symbol pairs) (:expression (q_atomic_state "allowed" (source target)))))
        (:rules (:rule (:symbol connect-pair) (:expression
          (:source-memory m0) (:target-memory m1)
          (:action (:conditions) (:action "connect") (:query pairs) (:effects)))))))""",
        parser.get_domain(), domain.ext_repository,
    )
    expander = getattr(ext, f"{kind.title()}SuccessorExpander")(task_context, program)
    state = expander.initial_state(initial_node(task_context).get_state())

    def arguments(action):
        return tuple(object_.get_name() for object_ in action.get_objects())

    search = task_context.search_context
    node = initial_node(task_context)
    broad = search.successor_generator.get_labeled_successor_nodes(
        node, search.state_repository, search.axiom_evaluator,
    )
    assert {arguments(successor.label) for successor in broad} == {
        ("a", "c"), ("a", "d"), ("b", "c"), ("b", "d"),
    }
    allowed = {("a", "c"), ("b", "d")}
    steps = collect_steps(expander, state)
    assert len(steps) == 2
    assert {arguments(step.state_transition.action) for step in steps} == allowed
    rule = program.get_entry_module().get_memory_transitions()[0][0]
    matched = set()
    applied = set()
    for successor in broad:
        if expander.matching_rule(state, successor) is not None:
            matched.add(arguments(successor.label))
        result = expander.apply(state, rule, successor)
        if result is not None:
            applied.add(arguments(result.state_transition.action))
    assert matched == applied == allowed
