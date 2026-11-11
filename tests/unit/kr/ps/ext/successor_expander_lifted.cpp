#include "successor_expander_fixtures.hpp"

#include <gtest/gtest.h>

namespace runir::tests
{

TEST(RunirTests, ExtInitialStatesUseExpanderRepositoryLifted) { expect_initial_program_state_uses_expander_repository<tyr::LiftedTag>(); }

TEST(RunirTests, ExtTransientProgramStatesRetainValuesAndInternCallersLifted)
{ expect_transient_program_states_retain_values_and_intern_callers<tyr::LiftedTag>(); }

TEST(RunirTests, ExtModuleReturnsPreserveCalleePlanningStateLifted) { expect_module_return_preserves_callee_planning_state<tyr::LiftedTag>(); }

TEST(RunirTests, ExtQueryEvaluationBorrowsIndependentSourceAndTargetCachesLifted) { expect_borrowed_query_evaluation<tyr::LiftedTag>(); }

TEST(RunirTests, ExtBindingEffectsFilterLoadsAndChoicesLifted) { expect_binding_effects_and_empty_choices<tyr::LiftedTag>(); }

TEST(RunirTests, ExtDoSuccessorsFilterArgumentsBeforeConstructingStatesLifted) { expect_lazy_do_successors<tyr::LiftedTag>(); }

TEST(RunirTests, ExtControlOnlyStepsDoNotGeneratePlanningSuccessorsLifted) { expect_control_only_steps_do_not_generate_planning_successors<tyr::LiftedTag>(); }

TEST(RunirTests, ExtQueryActionContractsLifted) { expect_query_action_contracts<tyr::LiftedTag>(); }

TEST(RunirTests, ExtUnknownActionRejectedDuringPreparationLifted)
{
    expect_unknown_action_rejected_during_preparation<tyr::LiftedTag, kr::ps::ext::ActionTag>();
    expect_unknown_action_rejected_during_preparation<tyr::LiftedTag, kr::ps::ext::DoTag>();
}

TEST(RunirTests, ExtCallbackSelectionStopsAfterSelectedRuleLifted) { expect_callback_selection_stops_after_selected_rule<tyr::LiftedTag>(); }

TEST(RunirTests, ExtRuleOccurrenceOrderLifted) { expect_rule_occurrence_order<tyr::LiftedTag>(); }

TEST(RunirTests, ExtCallbackSketchOrderAndCancellationLifted) { expect_callback_sketch_order_and_cancellation<tyr::LiftedTag>(); }

TEST(RunirTests, ExtQueryActionExistentialBindingLifted) { expect_query_action_existential_binding<tyr::LiftedTag>(); }

TEST(RunirTests, ExtQueryActionNullaryBindingLifted) { expect_query_action_nullary_binding<tyr::LiftedTag>(); }

TEST(RunirTests, ExtCallbackExpansionCountsLifted) { expect_callback_expansion_counts<tyr::LiftedTag>(); }

}  // namespace runir::tests
