#include "fixtures.hpp"
#include "planning_fixtures.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <runir/kr/dl/detail/repository_instantiations.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/evaluation.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/incremental/delta.hpp>
#include <runir/kr/dl/semantics/incremental/evaluation.hpp>
#include <runir/kr/dl/semantics/interning.hpp>
#include <runir/kr/dl/semantics/state_evaluation.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/domain_context.hpp>
#include <runir/kr/errors.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/module_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/feature_view.hpp>
#include <string>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <vector>

namespace runir::tests
{
namespace
{

auto query_search_context()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
}

/// Rules over the fixture: reachability from the markers along edges, the out-degree of reached objects, the reached
/// objects without a marker, and the first object of the triples that hold and are goals.
constexpr auto heads_text = R"(
(:module (:symbol entry) (:arguments) (:registers) (:entry m0) (:memory m0)
  (:datalog
    (:rule (reach ?x) (marker ?x))
    (:rule (reach ?y) (reach ?x) (edge ?x ?y))
    (:rule (degree ?x ?n) (reach ?x) (count ?n ((edge ?x ?y))))
    (:rule (unmarked ?x) (reach ?x) (not (marker ?x)))
    (:rule (achieved ?x) (triple ?x ?y ?z) (goal triple ?x ?y ?z)))
  (:features
    (:numerical (:symbol reached) (:expression (n_count (q_head reach (x)))))
    (:numerical (:symbol degrees) (:expression (n_count (q_head degree (x n)))))
    (:numerical (:symbol plain) (:expression (n_count (q_head unmarked (x)))))
    (:numerical (:symbol total) (:expression (n_add reached plain)))
    (:boolean (:symbol done) (:expression (b_nonempty (q_head achieved (x))))))
  (:rules))
)";

template<typename Module, typename Context>
std::vector<int64_t> values(Module module_, Context& context)
{
    auto result = std::vector<int64_t>();
    for (const auto feature : module_.template get_features<kr::dl::NumericalTag>())
        result.push_back(static_cast<int64_t>(kr::dl::semantics::evaluate<tyr::LiftedTag>(feature.get_feature(), context).get()));
    for (const auto feature : module_.template get_features<kr::dl::BooleanTag>())
        result.push_back(static_cast<int64_t>(kr::dl::semantics::evaluate<tyr::LiftedTag>(feature.get_feature(), context).get()));
    return result;
}

}  // namespace

TEST(RunirTests, DatalogHeadsAreFeatures)
{
    const auto search = query_search_context();
    const auto domain_context = kr::DomainContext::create(search->task->get_domain());
    const auto module_ = kr::ps::ext::dl::parse_module(heads_text, search->task->get_domain().get_domain(), *domain_context->ext_repository);

    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto context = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(initial.get_state(), builder, storage);
    // reach = {a, b, c}; degree has a row per reached object; unmarked = {b}; total = 3 + 1; (triple b c a) holds.
    EXPECT_EQ(values(module_, context), (std::vector<int64_t> { 3, 3, 1, 4, 1 }));

    // The incremental graph agrees with full evaluation along every successor and back.
    auto roots = std::vector<kr::dl::semantics::incremental::EvaluationRoot>();
    for (const auto feature : module_.get_features<kr::dl::NumericalTag>())
        roots.emplace_back(feature.get_feature());
    for (const auto feature : module_.get_features<kr::dl::BooleanTag>())
        roots.emplace_back(feature.get_feature());
    auto evaluation = kr::dl::semantics::incremental::IncrementalEvaluation<tyr::LiftedTag>(*search->task, roots);
    const auto incremental = [&](auto& state_evaluation)
    {
        auto result = std::vector<int64_t>();
        for (const auto feature : module_.get_features<kr::dl::NumericalTag>())
            result.push_back(static_cast<int64_t>(state_evaluation.evaluate(evaluation.graph().get_index(feature.get_feature())).get()));
        for (const auto feature : module_.get_features<kr::dl::BooleanTag>())
            result.push_back(static_cast<int64_t>(state_evaluation.evaluate(evaluation.graph().get_index(feature.get_feature())).get()));
        return result;
    };
    auto& workspace = builder.get_workspace().get_database_workspace();
    storage.reset_dynamic();
    auto first = evaluation.enter(context);
    EXPECT_EQ(incremental(first), (std::vector<int64_t> { 3, 3, 1, 4, 1 }));
    const auto successors = search->successor_generator->get_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator);
    ASSERT_FALSE(successors.empty());
    for (const auto& successor : successors)
    {
        auto delta = kr::dl::semantics::incremental::Delta {};
        delta.assign<tyr::LiftedTag>(initial.get_state(), successor.get_state());
        storage.reset_dynamic();
        auto successor_context = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(successor.get_state(), builder, storage);
        const auto expected = values(module_, successor_context);
        auto forward = evaluation.enter(successor_context, delta, workspace);
        EXPECT_EQ(incremental(forward), expected);
        for (const auto feature : module_.get_features<kr::dl::NumericalTag>())
            EXPECT_NO_THROW(forward.changed(evaluation.graph().get_index(feature.get_feature())));  // Updated from the delta.
        for (const auto feature : module_.get_features<kr::dl::BooleanTag>())
            EXPECT_NO_THROW(forward.changed(evaluation.graph().get_index(feature.get_feature())));
        delta.reverse();
        storage.reset_dynamic();
        auto back = evaluation.enter(context, delta, workspace);
        EXPECT_EQ(incremental(back), (std::vector<int64_t> { 3, 3, 1, 4, 1 }));
    }
}

TEST(RunirTests, DatalogRulesReadFeaturesInModules)
{
    // Rules read the concept feature `start`; features read heads and each other by name, in any order.
    const auto module_text = std::string(R"(
(:module (:symbol entry) (:arguments) (:registers) (:entry m0) (:memory m0)
  (:datalog
    (:rule (next ?y) (start ?x) (edge ?x ?y))
    (:rule (pair ?x ?y) (next ?x) (next ?y)))
  (:features
    (:numerical (:symbol reached) (:expression (n_count reachable)))
    (:numerical (:symbol any) (:expression (n_count both)))
    (:numerical (:symbol pair-count) (:expression (n_count pairs)))
    (:concept (:symbol start) (:expression (c_atomic_state "marker")))
    (:concept (:symbol reachable) (:expression (c_project x (q_head next (x)))))
    (:concept (:symbol both) (:expression (c_or start reachable)))
    (:query (:symbol pairs) (:expression (q_head pair (x y)))))
  (:rules))
)");
    const auto search = query_search_context();
    const auto domain_context = kr::DomainContext::create(search->task->get_domain());
    const auto module_ = kr::ps::ext::dl::parse_module(module_text, search->task->get_domain().get_domain(), *domain_context->ext_repository);
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto context = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(initial.get_state(), builder, storage);
    auto result = std::vector<int64_t>();
    for (const auto feature : module_.get_features<kr::dl::NumericalTag>())
        result.push_back(static_cast<int64_t>(kr::dl::semantics::evaluate<tyr::LiftedTag>(feature.get_feature(), context).get()));
    // start = {a, c} (markers); next = {b} (a -> b); both = {a, b, c}; pair = {(b, b)}.
    EXPECT_EQ(result, (std::vector<int64_t> { 1, 3, 1 }));

    // A rule cannot read a feature that reads the program's heads (one program per module).
    auto layered = module_text;
    layered.replace(layered.find("(start ?x) (edge"), 10, "(reachable ?x)");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(layered, search->task->get_domain().get_domain(), *domain_context->ext_repository), kr::InvalidExpressionError);
}

TEST(RunirTests, DatalogRulesComputeWithNumbers)
{
    // Out-degrees of the markers, compared with a number and doubled; arithmetic results are read by later literals.
    const auto module_text = std::string(R"(
(:module (:symbol entry) (:arguments) (:registers) (:entry m0) (:memory m0)
  (:datalog
    (:rule (degree ?x ?n) (marker ?x) (count ?n ((edge ?x ?y))))
    (:rule (busy ?x) (degree ?x ?n) (>= ?n 1))
    (:rule (idle ?x) (degree ?x ?n) (< ?n 1))
    (:rule (twice ?x ?m) (degree ?x ?n) (= ?m (* ?n 2)))
    (:rule (two ?x) (twice ?x ?m) (= ?m 2))
    (:rule (left ?x) (degree ?x ?n) (= ?k (- ?n 5)) (= ?k 0)))
  (:features
    (:numerical (:symbol busy-count) (:expression (n_count (q_head busy (x)))))
    (:numerical (:symbol idle-count) (:expression (n_count (q_head idle (x)))))
    (:numerical (:symbol twice-count) (:expression (n_count (q_head twice (x m)))))
    (:numerical (:symbol two-count) (:expression (n_count (q_head two (x)))))
    (:numerical (:symbol left-count) (:expression (n_count (q_head left (x))))))
  (:rules))
)");
    const auto search = query_search_context();
    const auto domain_context = kr::DomainContext::create(search->task->get_domain());
    const auto module_ = kr::ps::ext::dl::parse_module(module_text, search->task->get_domain().get_domain(), *domain_context->ext_repository);
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto context = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(initial.get_state(), builder, storage);
    // markers a, c; edges a -> b, b -> c: degree a = 1, c = 0; busy = {a}, idle = {c}, twice = {(a, 2), (c, 0)},
    // two = {a}; 1 - 5 and 0 - 5 saturate at 0: left = {a, c}.
    EXPECT_EQ(values(module_, context), (std::vector<int64_t> { 1, 1, 2, 1, 2 }));
}

TEST(RunirTests, DatalogWeightedHeadsKeepLeastWeights)
{
    // dist: unit distances from the markers, the least as its last column; far reads them in a higher stratum.
    const auto module_text = std::string(R"(
(:module (:symbol entry) (:arguments) (:registers) (:entry m0) (:memory m0)
  (:datalog
    (:rule (dist ?x (min 0)) (marker ?x))
    (:rule (dist ?y (min ?v)) (dist ?x ?w) (edge ?x ?y) (= ?v (+ ?w 1)))
    (:rule (far ?x) (dist ?x ?w) (>= ?w 1)))
  (:features
    (:numerical (:symbol reached) (:expression (n_count (q_head dist (x w)))))
    (:numerical (:symbol far-count) (:expression (n_count (q_head far (x))))))
  (:rules))
)");
    const auto search = query_search_context();
    const auto domain_context = kr::DomainContext::create(search->task->get_domain());
    const auto module_ = kr::ps::ext::dl::parse_module(module_text, search->task->get_domain().get_domain(), *domain_context->ext_repository);
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto context = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(initial.get_state(), builder, storage);
    // markers a, c (distance 0); a -> b: b at 1, and c stays 0 although b -> c; one row per object.
    EXPECT_EQ(values(module_, context), (std::vector<int64_t> { 3, 1 }));

    // A filter on a weight inside the minimizing rule is rejected, and so are rules of dist that disagree on min.
    auto filtered = module_text;
    filtered.replace(filtered.find("(edge ?x ?y) (= ?v"), 13, "(edge ?x ?y) (< ?w 3)");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(filtered, search->task->get_domain().get_domain(), *domain_context->ext_repository), kr::InvalidExpressionError);
    auto mixed = module_text;
    mixed.replace(mixed.find("(dist ?x (min 0))"), 17, "(dist ?x 0)");
    EXPECT_THROW(kr::ps::ext::dl::parse_module(mixed, search->task->get_domain().get_domain(), *domain_context->ext_repository), kr::InvalidExpressionError);
}

TEST(RunirTests, DatalogRulesReadRegistersByName)
{
    // Rules read the concept register `here` (one object), the role register `link` (one pair) and the concept
    // argument `from` as relations.
    const auto module_text = std::string(R"(
(:module (:symbol entry) (:arguments (:concept from)) (:registers (:concept here) (:role link)) (:entry m0) (:memory m0)
  (:datalog
    (:rule (near ?y) (here ?x) (edge ?x ?y))
    (:rule (linked ?x) (link ?x ?y) (marker ?y))
    (:rule (given ?x) (from ?x) (marker ?x)))
  (:features
    (:numerical (:symbol nears) (:expression (n_count (q_head near (y)))))
    (:numerical (:symbol links) (:expression (n_count (q_head linked (x)))))
    (:numerical (:symbol givens) (:expression (n_count (q_head given (x))))))
  (:rules))
)");
    const auto search = query_search_context();
    const auto domain_context = kr::DomainContext::create(search->task->get_domain());
    const auto module_ = kr::ps::ext::dl::parse_module(module_text, search->task->get_domain().get_domain(), *domain_context->ext_repository);
    const auto domain = search->task->get_domain().get_domain();
    const auto a = domain.get_constants()[0].get_index(), b = domain.get_constants()[1].get_index(), c = domain.get_constants()[2].get_index();
    auto register_data = std::array<ygg::Data<kr::dl::semantics::RegisterValues>, 2> {};
    for (auto& data : register_data)
    {
        data.concept_values.resize(1);
        data.role_values.resize(1);
    }
    // here = a, link = (b, c); then here = c, link = (b, b).
    register_data[0].concept_values[0] = a;
    register_data[0].role_values[0] = cista::pair(b, c);
    register_data[1].concept_values[0] = c;
    register_data[1].role_values[0] = cista::pair(b, b);
    const auto registers = [&](size_t i) { return ygg::make_view(register_data[i], *search->task->get_repository()); };

    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    // from = {a, b}: one marker.
    auto from = ygg::Builder<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(3);
    from.get().set(a.get_value());
    from.get().set(b.get_value());
    auto argument_data = ygg::Data<kr::dl::semantics::CallArguments>();
    argument_data.concept_arguments.push_back(kr::dl::semantics::insert(denotations, from, builder).first.get_index());
    const auto arguments = kr::dl::semantics::insert(denotations, argument_data).first;
    const auto context_for = [&](size_t i)
    { return kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(initial.get_state(), builder, storage, arguments, registers(i)); };
    // near = successors of `here`: {b} for a, {} for c; linked = first of `link` if its second is a marker: {b}, {};
    // given = the markers of `from`: {a}.
    const auto expected = std::array { std::vector<int64_t> { 1, 1, 1 }, std::vector<int64_t> { 0, 0, 1 } };
    for (size_t i = 0; i < 2; ++i)
    {
        storage.reset_dynamic();  // Before changing registers; a program that reads registers is dynamic.
        auto context = context_for(i);
        EXPECT_EQ(values(module_, context), expected[i]);
    }

    // Incremental evaluation sees a change of the registers alone.
    auto roots = std::vector<kr::dl::semantics::incremental::EvaluationRoot>();
    for (const auto feature : module_.get_features<kr::dl::NumericalTag>())
        roots.emplace_back(feature.get_feature());
    auto evaluation = kr::dl::semantics::incremental::IncrementalEvaluation<tyr::LiftedTag>(*search->task, roots);
    const auto incremental = [&](auto& state_evaluation)
    {
        auto result = std::vector<int64_t>();
        for (const auto feature : module_.get_features<kr::dl::NumericalTag>())
            result.push_back(static_cast<int64_t>(state_evaluation.evaluate(evaluation.graph().get_index(feature.get_feature())).get()));
        return result;
    };
    auto& workspace = builder.get_workspace().get_database_workspace();
    storage.reset_dynamic();
    auto first_context = context_for(0);
    auto first = evaluation.enter(first_context);
    EXPECT_EQ(incremental(first), expected[0]);
    auto delta = kr::dl::semantics::incremental::Delta {};
    delta.assign<tyr::LiftedTag>(initial.get_state(), registers(0), initial.get_state(), registers(1));
    storage.reset_dynamic();
    auto second_context = context_for(1);
    auto second = evaluation.enter(second_context, delta, workspace);
    EXPECT_EQ(incremental(second), expected[1]);
}

TEST(RunirTests, DatalogRulesReadBooleanAndNumericalArgumentsAndFeatures)
{
    // Rules read the Boolean argument `on` (a nullary relation), the numerical argument `bound`, the numerical
    // feature `triples` (one row with the value) and the Boolean feature `ready` (nullary).
    const auto module_text = std::string(R"(
(:module (:symbol entry) (:arguments (:boolean on) (:numerical bound)) (:registers) (:entry m0) (:memory m0)
  (:datalog
    (:rule (limit ?v) (bound ?b) (= ?v (+ ?b 1)))
    (:rule (within) (triples ?m) (bound ?b) (<= ?m ?b))
    (:rule (gated) (ready) (on)))
  (:features
    (:numerical (:symbol triples) (:expression (n_count (q_atomic_state "triple" (x y z)))))
    (:boolean (:symbol ready) (:expression (b_atomic_state "ready" true)))
    (:numerical (:symbol limits) (:head limit))
    (:boolean (:symbol withins) (:head within))
    (:boolean (:symbol gates) (:head gated)))
  (:rules))
)");
    const auto search = query_search_context();
    const auto domain_context = kr::DomainContext::create(search->task->get_domain());
    const auto module_ = kr::ps::ext::dl::parse_module(module_text, search->task->get_domain().get_domain(), *domain_context->ext_repository);
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    const auto arguments = [&](bool on, ygg::uint_t bound)
    {
        auto boolean = ygg::Data<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>();
        boolean.value = on;
        auto numerical = ygg::Data<kr::dl::semantics::Denotation<kr::dl::NumericalTag>>();
        numerical.value = bound;
        auto data = ygg::Data<kr::dl::semantics::CallArguments>();
        data.boolean_arguments.push_back(denotations.insert(boolean).first.get_index());
        data.numerical_arguments.push_back(denotations.insert(numerical).first.get_index());
        return kr::dl::semantics::insert(denotations, data).first;
    };
    const auto register_data = ygg::Data<kr::dl::semantics::RegisterValues>();
    const auto registers = ygg::make_view(register_data, *search->task->get_repository());
    auto roots = std::vector<kr::dl::semantics::incremental::EvaluationRoot>();
    for (const auto feature : module_.get_features<kr::dl::NumericalTag>())
        roots.emplace_back(feature.get_feature());
    for (const auto feature : module_.get_features<kr::dl::BooleanTag>())
        roots.emplace_back(feature.get_feature());
    auto evaluation = kr::dl::semantics::incremental::IncrementalEvaluation<tyr::LiftedTag>(*search->task, roots);
    for (const auto& [on, bound] : { std::pair { true, ygg::uint_t { 0 } }, std::pair { false, ygg::uint_t { 5 } } })
    {
        storage.reset_dynamic();
        const auto call = arguments(on, bound);
        auto context = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(initial.get_state(), builder, storage, call, registers);
        const auto result = values(module_, context);
        // triples, limits; ready, withins, gates: four triples and ready in the initial state.
        EXPECT_EQ(result, (std::vector<int64_t> { 4, static_cast<int64_t>(bound) + 1, 1, 4 <= static_cast<int64_t>(bound), on }));
        storage.reset_dynamic();
        auto state_evaluation = evaluation.enter(context);
        auto incremental = std::vector<int64_t>();
        for (const auto feature : module_.get_features<kr::dl::NumericalTag>())
            incremental.push_back(static_cast<int64_t>(state_evaluation.evaluate(evaluation.graph().get_index(feature.get_feature())).get()));
        for (const auto feature : module_.get_features<kr::dl::BooleanTag>())
            incremental.push_back(static_cast<int64_t>(state_evaluation.evaluate(evaluation.graph().get_index(feature.get_feature())).get()));
        EXPECT_EQ(incremental, result);
    }
    {
        // Removing triples one by one, incremental evaluation agrees with full evaluation as `triples` and `ready` change.
        const auto call = arguments(true, 1);
        const auto incremental = [&](auto& state_evaluation)
        {
            auto result = std::vector<int64_t>();
            for (const auto feature : module_.get_features<kr::dl::NumericalTag>())
                result.push_back(static_cast<int64_t>(state_evaluation.evaluate(evaluation.graph().get_index(feature.get_feature())).get()));
            for (const auto feature : module_.get_features<kr::dl::BooleanTag>())
                result.push_back(static_cast<int64_t>(state_evaluation.evaluate(evaluation.graph().get_index(feature.get_feature())).get()));
            return result;
        };
        auto& workspace = builder.get_workspace().get_database_workspace();
        storage.reset_dynamic();
        auto context = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(initial.get_state(), builder, storage, call, registers);
        auto first = evaluation.enter(context);
        static_cast<void>(incremental(first));
        auto changed = false;
        auto node = initial;
        for (int step = 0; step < 4; ++step)
        {
            const auto successors = search->successor_generator->get_successor_nodes(node, *search->state_repository, *search->axiom_evaluator);
            ASSERT_FALSE(successors.empty());
            const auto& next = successors.front();
            auto delta = kr::dl::semantics::incremental::Delta {};
            delta.assign<tyr::LiftedTag>(node.get_state(), next.get_state());
            storage.reset_dynamic();
            auto next_context = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(next.get_state(), builder, storage, call, registers);
            const auto expected = values(module_, next_context);
            storage.reset_dynamic();
            auto forward = evaluation.enter(next_context, delta, workspace);
            EXPECT_EQ(incremental(forward), expected) << "step " << step;
            changed = changed || expected[3] != 0;
            node = next;
        }
        EXPECT_TRUE(changed);  // `within` became true: at most one triple is left.
    }
}

TEST(RunirTests, DatalogRejectsInvalidPrograms)
{
    const auto search = query_search_context();
    const auto domain = search->task->get_domain().get_domain();
    const auto parse = [&](const std::string& datalog, const std::string& features)
    {
        const auto domain_context = kr::DomainContext::create(search->task->get_domain());
        return kr::ps::ext::dl::parse_module("(:module (:symbol m) (:arguments) (:registers) (:entry m0) (:memory m0) (:datalog " + datalog + ") (:features "
                                                 + features + ") (:rules))",
                                             domain,
                                             *domain_context->ext_repository);
    };
    const auto read = std::string("(:numerical (:symbol f) (:expression (n_count (q_head r (x)))))");
    EXPECT_THROW(parse("(:rule (r ?x) (unknown ?x))", read), kr::UndefinedSymbolError);
    EXPECT_THROW(parse("(:rule (r ?x) (edge ?x))", read), kr::ArityMismatchError);
    EXPECT_THROW(parse("(:rule (r ?x) (marker ?y))", read), kr::InvalidExpressionError);  // unsafe
    EXPECT_THROW(parse("(:rule (r ?x) (marker ?x)) (:rule (r ?x ?y) (edge ?x ?y))", read), kr::ArityMismatchError);
    EXPECT_THROW(parse("(:rule (r ?x) (marker ?x))", "(:numerical (:symbol f) (:expression (n_count (q_head r (x y)))))"), kr::InvalidExpressionError);
    EXPECT_THROW(parse("(:rule (r ?x) (marker ?x))", "(:numerical (:symbol r) (:expression (n_count (q_head r (x)))))"), kr::DuplicateDefinitionError);
    EXPECT_THROW(parse("(:rule (r ?x) (marker ?x))",
                       "(:numerical (:symbol f) (:expression (n_add g (n_const 1)))) (:numerical (:symbol g) (:expression (n_add f (n_const 1))))"),
                 kr::InvalidExpressionError);  // cycle
    // Syntax the grammar rejects: a nested aggregate, a negation without an atom, a count with a value variable.
    EXPECT_THROW(parse("(:rule (r ?x) (marker ?x) (count ?n ((count ?m ((edge ?x ?y))))))", read), kr::ParseError);
    EXPECT_THROW(parse("(:rule (r ?x) (marker ?x) (not))", read), kr::ParseError);
    EXPECT_THROW(parse("(:rule (r ?x) (marker ?x) (count ?n ?v ((edge ?x ?v))))", read), kr::InvalidExpressionError);
    // A section that no feature reads is still checked.
    EXPECT_THROW(parse("(:rule (r ?x) (unknown ?x))", "(:numerical (:symbol f) (:expression (n_const 1)))"), kr::UndefinedSymbolError);
    EXPECT_NO_THROW(parse("(:rule (r ?x) (marker ?x))", read));
    // A Boolean head feature reads a nullary head, a numerical one a unary head.
    EXPECT_THROW(parse("(:rule (r ?x) (marker ?x))", "(:boolean (:symbol f) (:head r))"), kr::InvalidExpressionError);
    EXPECT_THROW(parse("(:rule (r ?x ?y) (edge ?x ?y))", "(:numerical (:symbol f) (:head r))"), kr::InvalidExpressionError);
    EXPECT_THROW(parse("(:rule (r ?x) (marker ?x))", "(:numerical (:symbol f) (:head s))"), kr::UndefinedSymbolError);
    EXPECT_THROW(parse("(:rule (r ?x) (marker ?x))", "(:numerical (:symbol r) (:head r))"), kr::DuplicateDefinitionError);
    EXPECT_NO_THROW(parse("(:rule (r) (marker ?x)) (:rule (n (min 1)) (r))", "(:boolean (:symbol f) (:head r)) (:numerical (:symbol g) (:head n))"));
    // Sketches have no :datalog section, so q_head and feature names are not supported there.
    const auto domain_context = kr::DomainContext::create(search->task->get_domain());
    EXPECT_THROW(kr::ps::base::dl::parse_sketch("(:sketch (:features " + read + ") (:rules))", domain, *domain_context->base_repository),
                 kr::InvalidExpressionError);
}

/// The ramp heights still needed for the goal of Autoscale Termes p16, per state of a random walk, with full and with
/// incremental evaluation: both agree, and their times are printed.
TEST(RunirTests, DatalogDistanceEqualsNDistanceOnTermes)
{
    // The robot's distance to the depot over NEIGHBOR, as n_distance and as weighted rules with a minimizing goal head over the depots
    // read by a numerical head feature, along a random walk of Autoscale Termes p16, with full and incremental evaluation; both are
    // infinity (the largest uint_t) where no depot is reachable, which a Boolean head feature tells.
    const auto directory = benchmark_path("classical/autoscale-benchmarks-main/21.11-agile-strips/termes");
    const auto search = make_lifted_context(directory / "domain.pddl", directory / "p16.pddl");
    const auto domain_context = kr::DomainContext::create(search->task->get_domain());
    const auto module_ = kr::ps::ext::dl::parse_module(R"(
(:module (:symbol entry) (:arguments) (:registers) (:entry m0) (:memory m0)
  (:datalog
    (:rule (dist ?p (min 0)) (at ?p))
    (:rule (dist ?q (min ?v)) (dist ?p ?w) (neighbor ?p ?q) (= ?v (+ ?w 1)))
    (:rule (best (min ?w)) (dist ?p ?w) (is-depot ?p))
    (:rule (reached) (best ?w)))
  (:features
    (:numerical (:symbol to-depot) (:expression (n_distance (c_atomic_state "at") (r_atomic_state "neighbor") (c_atomic_state "is-depot"))))
    (:numerical (:symbol closest) (:head best))
    (:boolean (:symbol reachable) (:head reached)))
  (:rules))
)",
                                                       search->task->get_domain().get_domain(),
                                                       *domain_context->ext_repository);
    const auto distance = module_.get_features<kr::dl::NumericalTag>()[0].get_feature();
    const auto best = module_.get_features<kr::dl::NumericalTag>()[1].get_feature();
    const auto reachable = module_.get_features<kr::dl::BooleanTag>()[0].get_feature();
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto evaluation = kr::dl::semantics::incremental::IncrementalEvaluation<tyr::LiftedTag>(*search->task, { best, reachable });
    auto& workspace = builder.get_workspace().get_database_workspace();

    auto random = std::mt19937(1);
    auto node = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    {
        auto context = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(node.get_state(), builder, storage);
        static_cast<void>(evaluation.enter(context).evaluate(evaluation.graph().get_index(best)));
    }
    auto rules = std::vector<double>(), constructor = std::vector<double>();
    const auto seconds = [](auto start) { return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count(); };
    for (int step = 0; step < 300; ++step)
    {
        const auto successors = search->successor_generator->get_successor_nodes(node, *search->state_repository, *search->axiom_evaluator);
        const auto& next = successors[random() % successors.size()];
        auto delta = kr::dl::semantics::incremental::Delta {};
        delta.assign<tyr::LiftedTag>(node.get_state(), next.get_state());
        storage.reset_dynamic();
        auto context = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(next.get_state(), builder, storage);
        auto start = std::chrono::steady_clock::now();
        const auto expected = kr::dl::semantics::evaluate<tyr::LiftedTag>(distance, context).get();
        constructor.push_back(seconds(start));
        storage.reset_dynamic();
        start = std::chrono::steady_clock::now();
        const auto value = kr::dl::semantics::evaluate<tyr::LiftedTag>(best, context).get();
        rules.push_back(seconds(start));
        ASSERT_EQ(value, expected) << "step " << step;
        ASSERT_EQ(kr::dl::semantics::evaluate<tyr::LiftedTag>(reachable, context).get(), expected != std::numeric_limits<ygg::uint_t>::max());
        storage.reset_dynamic();
        auto incremental = evaluation.enter(context, delta, workspace);
        ASSERT_EQ(incremental.evaluate(evaluation.graph().get_index(best)).get(), expected) << "step " << step;
        ASSERT_EQ(incremental.evaluate(evaluation.graph().get_index(reachable)).get(), expected != std::numeric_limits<ygg::uint_t>::max());
        node = next;
    }
    const auto report = [](const char* name, std::vector<double> times)
    {
        std::ranges::sort(times);
        const auto mean = std::accumulate(times.begin(), times.end(), 0.0) / static_cast<double>(times.size());
        std::cout << name << ": median " << times[times.size() / 2] * 1e6 << " us, mean " << mean * 1e6 << " us\n";
    };
    report("n_distance (full)", constructor);
    report("weighted rules (full)", rules);
}

TEST(RunirTests, DatalogTermesTiming)
{
    const auto directory = benchmark_path("classical/autoscale-benchmarks-main/21.11-agile-strips/termes");
    const auto search = make_lifted_context(directory / "domain.pddl", directory / "p16.pddl");
    const auto domain_context = kr::DomainContext::create(search->task->get_domain());
    const auto module_ = kr::ps::ext::dl::parse_module(R"(
(:module (:symbol entry) (:arguments) (:registers) (:entry m0) (:memory m0)
  (:datalog
    (:rule (built ?p ?h) (height ?p ?h))
    (:rule (built ?p ?l) (built ?p ?h) (succ ?h ?l))
    (:rule (need ?p ?h) (goal height ?p ?h))
    (:rule (need ?n ?l) (need ?p ?h) (neighbor ?n ?p) (succ ?h ?l) (not (built ?n ?l))))
  (:features (:numerical (:symbol needed) (:expression (n_count (q_head need (p h))))))
  (:rules))
)",
                                                       search->task->get_domain().get_domain(),
                                                       *domain_context->ext_repository);
    const auto feature = module_.get_features<kr::dl::NumericalTag>()[0].get_feature();
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto evaluation = kr::dl::semantics::incremental::IncrementalEvaluation<tyr::LiftedTag>(*search->task, { feature });
    const auto index = evaluation.graph().get_index(feature);
    auto& workspace = builder.get_workspace().get_database_workspace();

    auto random = std::mt19937(0);
    auto node = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto full = std::vector<double>(), incremental = std::vector<double>();
    const auto seconds = [](auto start) { return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count(); };
    {
        storage.reset_all();
        auto context = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(node.get_state(), builder, storage);
        static_cast<void>(evaluation.enter(context).evaluate(index));
    }
    for (int step = 0; step < 1000; ++step)
    {
        const auto successors = search->successor_generator->get_successor_nodes(node, *search->state_repository, *search->axiom_evaluator);
        const auto& next = successors[random() % successors.size()];
        auto delta = kr::dl::semantics::incremental::Delta {};
        delta.assign<tyr::LiftedTag>(node.get_state(), next.get_state());
        storage.reset_dynamic();
        auto context = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>(next.get_state(), builder, storage);
        auto start = std::chrono::steady_clock::now();
        const auto expected = kr::dl::semantics::evaluate<tyr::LiftedTag>(feature, context).get();
        full.push_back(seconds(start));
        storage.reset_dynamic();
        start = std::chrono::steady_clock::now();
        const auto value = evaluation.enter(context, delta, workspace).evaluate(index).get();
        incremental.push_back(seconds(start));
        ASSERT_EQ(value, expected);
        node = next;
    }
    const auto report = [](const char* name, std::vector<double> times)
    {
        std::ranges::sort(times);
        const auto mean = std::accumulate(times.begin(), times.end(), 0.0) / static_cast<double>(times.size());
        std::cout << name << ": median " << times[times.size() / 2] * 1e6 << " us, mean " << mean * 1e6 << " us, max " << times.back() * 1e6 << " us\n";
    };
    report("full evaluation", full);
    report("incremental evaluation", incremental);
}

}  // namespace runir::tests
