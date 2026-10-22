#include "planning_fixtures.hpp"

#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <memory>
#include <ranges>
#include <runir/kr/dl/detail/repository_instantiations.hpp>
#include <runir/kr/dl/optimization.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/evaluation.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/state_evaluation.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <string>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <vector>

namespace runir::tests
{
namespace
{

template<tyr::TaskKind Kind>
void check_optimized_queries()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    const auto search = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
        else
            return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
    }();
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());

    // Sample states: the initial state and its successors (removing triples).
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto states = std::vector { initial.get_state() };
    for (const auto& successor : search->successor_generator->get_labeled_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator))
        states.push_back(successor.node.get_state());

    using Context = kr::dl::semantics::StateEvaluationContext<Kind>;
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto empty_arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto register_data = ygg::Data<kr::dl::semantics::RegisterValues>();
    register_data.concept_values.resize(1);
    register_data.role_values.resize(1);
    const auto arguments = kr::dl::semantics::insert(denotations, empty_arguments).first;
    const auto registers = kr::dl::semantics::insert(denotations, register_data).first;
    // One storage per state, so dynamic results are never shared across states.
    auto storages = std::vector<std::unique_ptr<kr::dl::semantics::EvaluationStorage>>();
    auto contexts = std::vector<std::unique_ptr<Context>>();
    for (const auto& state : states)
    {
        storages.push_back(std::make_unique<kr::dl::semantics::EvaluationStorage>(denotations));
        contexts.push_back(std::make_unique<Context>(state, builder, *storages.back(), arguments, registers));
    }
    auto context_pointers = std::vector<Context*>();
    for (const auto& context : contexts)
        context_pointers.push_back(context.get());

    const auto triple = std::string(R"((q_atomic_state "triple" (x y z)))");
    const auto fixed = std::string(R"((q_atomic_state "fixed" (x y z)))");
    const auto edges = std::string(R"((r_or (r_atomic_state "edge") (r_inverse (r_atomic_state "edge"))))");
    const auto triangle = "(n_count (q_join (q_join (q_role (x y) " + edges + ") (q_role (y z) " + edges + ")) (q_role (x z) " + edges + ")))";
    // Edges evaluated per vertex keep their joins: planning them for full evaluation would add semijoins.
    const auto path = "(q_join (q_role (x z) " + edges + ") (q_join (q_role (z w) " + edges + ") (q_role (w y) " + edges + ")))";
    const auto distance = "(n_distance (q_concept x (c_atomic_state \"marker\")) (q_project (x y) " + path + ") (q_concept y (c_atomic_state \"marker\")))";
    const auto joins = [](const std::string& formatted)
    {
        size_t result = 0;
        for (auto position = formatted.find("(q_join"); position != std::string::npos; position = formatted.find("(q_join", position + 1))
            ++result;
        return result;
    };
    const auto expressions = std::vector<std::string> {
        triangle,
        R"((n_count (q_project (x) (q_join (q_atomic_state "assignment" (x y z w)) (q_atomic_state "required" (x y z))))))",
        "(n_count (q_join " + triple + R"( (q_role (x y) (r_atomic_state "edge")))))",
        "(n_count (q_select_value x \"a\" (q_join " + triple + " " + fixed + ")))",
        "(n_count (q_union " + triple + " " + fixed + "))",
        // A query nested inside a leaf's concept argument is optimized too.
        "(n_count (c_project x (q_join " + triple + " (q_concept x (c_project z (q_join " + triple + " " + fixed + "))))))",
        "(n_count (q_generic_join (x y z) (x y z) " + triple + " " + fixed + "))",
        distance,
    };

    {
        // A leaf empty in every sampled state keeps a size of zero and is reported.
        const auto never = std::string(R"((q_atomic_state "missing" ()))");
        const auto original = kr::ps::ext::dl::parse_numerical("(n_count (q_join " + triple + " " + never + "))", domain, *repository);
        auto warnings = ygg::diagnostics::Warnings();
        const auto statistics = kr::dl::collect_query_statistics<Kind>(original, context_pointers, warnings);
        ASSERT_EQ(warnings.get().size(), 1);
        EXPECT_NE(warnings.get()[0].message.find(never + " was empty in all"), std::string::npos) << warnings.get()[0].message;
        const auto optimized = kr::dl::optimize(original, *repository, statistics);
        for (auto* context : context_pointers)
            EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(optimized, *context).get(), kr::dl::semantics::evaluate<Kind>(original, *context).get());
    }

    for (const auto& text : expressions)
    {
        const auto original = kr::ps::ext::dl::parse_numerical(text, domain, *repository);
        auto warnings = ygg::diagnostics::Warnings();
        const auto statistics = kr::dl::collect_query_statistics<Kind>(original, context_pointers, warnings);
        EXPECT_TRUE(warnings.empty()) << text;
        EXPECT_TRUE(statistics.objects.has_value()) << text;
        EXPECT_FALSE(statistics.leaves.empty()) << text;
        {
            // Measuring one state at a time, in any order, gives the statistics of the batch.
            auto collector = kr::dl::QueryStatisticsCollector();
            collector.add(original);
            auto sampler = collector.sampler();
            for (auto* context : std::views::reverse(context_pointers))
                sampler.template add<Kind>(*context);
            const auto sampled = sampler.finish(warnings);
            EXPECT_EQ(sampled.objects, statistics.objects) << text;
            ASSERT_EQ(sampled.leaves.size(), statistics.leaves.size()) << text;
            for (const auto& [leaf, expected] : statistics.leaves)
            {
                const auto it = sampled.leaves.find(leaf);
                ASSERT_NE(it, sampled.leaves.end()) << text;
                EXPECT_EQ(it->second.rows, expected.rows) << text;
                EXPECT_EQ(it->second.distinct, expected.distinct) << text;
            }
        }
        for (const auto* mode : { &statistics, static_cast<const kr::dl::QueryStatistics*>(nullptr) })
        {
            const auto optimized = mode ? kr::dl::optimize(original, *repository, *mode) : kr::dl::optimize(original, *repository);
            const auto formatted = fmt::format("{}", optimized);
            EXPECT_EQ(kr::ps::ext::dl::parse_numerical(formatted, domain, *repository), optimized) << formatted;
            for (auto* context : context_pointers)
                EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(optimized, *context).get(), kr::dl::semantics::evaluate<Kind>(original, *context).get())
                    << text << " => " << formatted;
            if (text == triangle)
            {
                EXPECT_NE(formatted.find("q_generic_join"), std::string::npos) << formatted;
            }
            if (text == distance)
            {
                EXPECT_EQ(joins(formatted), joins(text)) << formatted;
            }
        }
    }
}

}  // namespace

TEST(RunirQueryOptimization, GroundOptimizedFeaturesEvaluateLikeOriginals) { check_optimized_queries<tyr::GroundTag>(); }
TEST(RunirQueryOptimization, LiftedOptimizedFeaturesEvaluateLikeOriginals) { check_optimized_queries<tyr::LiftedTag>(); }

}  // namespace runir::tests
