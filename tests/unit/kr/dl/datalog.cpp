#include "fixtures.hpp"
#include "planning_fixtures.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
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
    // Sketches have no :datalog section, so q_head and feature names are not supported there.
    const auto domain_context = kr::DomainContext::create(search->task->get_domain());
    EXPECT_THROW(kr::ps::base::dl::parse_sketch("(:sketch (:features " + read + ") (:rules))", domain, *domain_context->base_repository),
                 kr::InvalidExpressionError);
}

/// The ramp heights still needed for the goal of Autoscale Termes p16, per state of a random walk, with full and with
/// incremental evaluation: both agree, and their times are printed.
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
