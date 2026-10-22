import pytest
from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pytyr.formalism.planning import Parser
from pytyr.planning import lifted
from pyyggdrasil.execution import ExecutionContext

from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext, LiftedTaskContext
from pyrunir.kr.dl import semantics
from pyrunir.kr.ps import icp
from pyrunir.kr.ps.base.dl import parse_sketch
from pyrunir.kr.ps.ext.dl import parse_program
from pyrunir.kr.uns import classify
from pyrunir.kr.uns.dl import parse_classifier

# Optimization replans a generic join over two atoms as a binary join, so every object below changes.
_LOCATED = '(q_generic_join (ball room) (ball room) (q_atomic_state "at" (ball room)) (q_concept ball (c_atomic_state "ball")))'
_GOALS = '(q_generic_join (ball room) (ball room) (q_atomic_goal "at" true (ball room)) (q_concept ball (c_atomic_state "ball")))'


def _ground(gripper_planning_domain, ground_gripper_search_context):
    domain = DomainContext(gripper_planning_domain)
    task = GroundTaskContext(domain, ground_gripper_search_context)
    state = ground_gripper_search_context.state_repository.get_initial_state(ground_gripper_search_context.axiom_evaluator)
    return domain, task, state


def test_base_sketch(gripper_planning_domain, ground_gripper_search_context):
    domain, task, state = _ground(gripper_planning_domain, ground_gripper_search_context)
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, semantics.EvaluationStorage(task.dl_denotation_repository))
    source = (
        f"(:sketch (:features (:numerical (:symbol located) (:expression (n_count {_LOCATED})))) "
        "(:rules (:rule (:symbol step) (:expression (:conditions) (:effects (decreases located))))))"
    )
    sketch = parse_sketch(source, gripper_planning_domain, domain.base_repository)
    statistics = sketch.collect_query_statistics([context])
    assert statistics.num_leaves() == 2
    for optimized in (sketch.optimize(domain.base_repository), sketch.optimize(domain.base_repository, statistics)):
        assert optimized != sketch
        # Rules must refer to the rewritten features, or printing and parsing would not round-trip.
        assert parse_sketch(str(optimized), gripper_planning_domain, domain.base_repository) == optimized
        before = sketch.get_numerical_features()[0].get_expression().evaluate(context).get()
        after = optimized.get_numerical_features()[0].get_expression().evaluate(context).get()
        assert before == after
    plain = parse_sketch("(:sketch (:features) (:rules))", gripper_planning_domain, domain.base_repository)
    assert plain.optimize(domain.base_repository) == plain


def test_statistics_warn_about_leaves_empty_in_every_sample(gripper_planning_domain, ground_gripper_search_context):
    domain, task, state = _ground(gripper_planning_domain, ground_gripper_search_context)
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, semantics.EvaluationStorage(task.dl_denotation_repository))
    carried = '(q_atomic_state "carry" (ball gripper))'  # no ball is carried in the initial state
    sketch = parse_sketch(
        f"(:sketch (:features (:numerical (:symbol carried) (:expression (n_count (q_join {carried} (q_concept ball (c_atomic_state \"ball\"))))))) (:rules))",
        gripper_planning_domain,
        domain.base_repository,
    )
    with pytest.warns(UserWarning, match="was empty in all 1 samples"):
        statistics = sketch.collect_query_statistics([context])
    assert statistics.num_leaves() == 2
    optimized = sketch.optimize(domain.base_repository, statistics)
    assert optimized.get_numerical_features()[0].get_expression().evaluate(context).get() == 0


def test_ext_program(gripper_planning_domain, ground_gripper_search_context):
    domain, task, state = _ground(gripper_planning_domain, ground_gripper_search_context)
    bindings = (
        task.dl_denotation_repository.insert(semantics.CallArgumentsData())[0],
        task.dl_denotation_repository.insert(semantics.RegisterValuesData())[0],
    )
    context = semantics.GroundStateEvaluationContext(
        state, task.dl_builder, semantics.EvaluationStorage(task.dl_denotation_repository), *bindings
    )
    source = (
        "(:program (:entry root) (:module (:symbol root) (:arguments) (:registers) (:entry m0) (:memory m0) "
        f"(:features (:numerical (:symbol located) (:expression (n_count {_LOCATED})))) "
        "(:rules (:rule (:symbol step) (:expression (:source-memory m0) (:target-memory m0) "
        "(:sketch (:conditions) (:effects (decreases located))))))))"
    )
    program = parse_program(source, gripper_planning_domain, domain.ext_repository)
    statistics = program.collect_query_statistics([context])
    assert statistics.num_leaves() == 2
    for optimized in (program.optimize(domain.ext_repository), program.optimize(domain.ext_repository, statistics)):
        assert optimized != program
        # The rule lives in the module's memory transitions; a missed rule would keep the old feature.
        assert parse_program(str(optimized), gripper_planning_domain, domain.ext_repository) == optimized
        before = program.get_entry_module().get_numerical_features()[0].get_expression().evaluate(context).get()
        after = optimized.get_entry_module().get_numerical_features()[0].get_expression().evaluate(context).get()
        assert before == after
    module = program.get_entry_module()
    assert module.optimize(domain.ext_repository) != module
    # Sampling one state at a time gives the statistics of the batch call.
    sampler = program.query_statistics_sampler()
    sampler.add(context)
    sampled = sampler.finish()
    assert sampled.num_leaves() == statistics.num_leaves()
    assert program.optimize(domain.ext_repository, sampled) == program.optimize(domain.ext_repository, statistics)


_ICP_PROGRAM = f"""(:program (:entry main)
  (:module (:symbol main) (:arguments) (:registers (:concept selected))
    (:entry start) (:memory start pick move drop done)
    (:features
      (:concept (:symbol candidates) (:expression (c_project ball {_GOALS})))
      (:concept (:symbol selected) (:expression (c_register selected)))
      (:concept (:symbol carried) (:expression (c_some (r_atomic_state "carry") (c_top))))
      (:concept (:symbol robot) (:expression (c_atomic_state "at-robby")))
      (:concept (:symbol delivered) (:expression
        (c_some (r_and (r_atomic_state "at") (r_atomic_goal "at" true)) (c_top))))
      (:numerical (:symbol selected_count) (:expression (n_count (c_register selected))))
      (:numerical (:symbol carried_count) (:expression (n_count (c_some (r_atomic_state "carry") (c_top))))))
    (:rules
      (:rule (:symbol load) (:expression (:source-memory start) (:target-memory pick)
        (:load (:conditions) (:concept candidates) (:register (:concept selected))
          (:effects (increases selected_count)))))
      (:rule (:symbol pick) (:expression (:source-memory pick) (:target-memory move)
        (:crule (:action "pick") (:arguments ball room gripper) (:conditions)
          (:xconditions (belongs (:argument ball) (:concept selected)))
          (:xeffects (enter (:argument ball) (:concept carried)))
          (:effects (increases carried_count)))))
      (:rule (:symbol move) (:expression (:source-memory move) (:target-memory drop)
        (:crule (:action "move") (:arguments from to) (:conditions)
          (:xconditions) (:xeffects (enter (:argument to) (:concept robot))) (:effects))))
      (:rule (:symbol drop) (:expression (:source-memory drop) (:target-memory done)
        (:crule (:action "drop") (:arguments ball room gripper) (:conditions)
          (:xconditions (belongs (:argument ball) (:concept selected)))
          (:xeffects (enter (:argument ball) (:concept delivered)))
          (:effects (decreases carried_count))))))
    (:reset-spo)))
"""


@pytest.mark.parametrize("kind", ["Ground", "Lifted"])
def test_icp_program(kind):
    directory = data_root() / "classical" / "tests" / "gripper"
    parser = Parser(directory / "domain.pddl", ParserOptions())
    task = lifted.Task(parser.parse_task(directory / "test-1.pddl", ParserOptions()))
    execution = ExecutionContext(1)
    domain = DomainContext(parser.get_domain())
    if kind == "Ground":
        context = GroundTaskContext(domain, GroundTaskSearchContext(task.instantiate_ground_task(execution).task, execution))
    else:
        context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
    program = icp.dl.parse_program(_ICP_PROGRAM, parser.get_domain(), domain.icp_repository)
    optimized = program.optimize(domain.icp_repository)
    assert optimized != program
    assert icp.dl.parse_program(str(optimized), parser.get_domain(), domain.icp_repository) == optimized
    options = getattr(icp, f"{kind}ProgramSearchOptions")()
    for candidate in (program, optimized):
        result = getattr(icp, f"find_{kind.lower()}_solution")(context, candidate, options)
        assert result.status == icp.ProgramProofStatus.SUCCESS


def test_uns_classifier(gripper_planning_domain, ground_gripper_search_context):
    domain, task, state = _ground(gripper_planning_domain, ground_gripper_search_context)
    context = semantics.GroundStateEvaluationContext(
        state, task.dl_builder, semantics.EvaluationStorage(task.dl_denotation_repository)
    )
    source = (
        f"(:classifier (:symbol root) (:features (:boolean (:symbol located) (:expression (b_nonempty {_LOCATED})))) "
        "(:expression (or (and located))))"
    )
    classifier = parse_classifier(source, gripper_planning_domain, domain.uns_repository)
    statistics = classifier.collect_query_statistics([context])
    assert statistics.num_leaves() == 2
    for optimized in (classifier.optimize(domain.uns_repository), classifier.optimize(domain.uns_repository, statistics)):
        assert optimized != classifier
        assert parse_classifier(str(optimized), gripper_planning_domain, domain.uns_repository) == optimized
        assert classify(optimized, context) == classify(classifier, context)
