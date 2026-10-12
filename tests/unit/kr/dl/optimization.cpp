#include "planning_fixtures.hpp"

#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <memory>
#include <runir/kr/dl/optimization.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/evaluation.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
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
using Ext = kr::ExtFamilyTag;

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
    auto repository = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());

    // Sample states: the initial state and its successors (removing triples).
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto states = std::vector { initial.get_state() };
    for (const auto& successor : search->successor_generator->get_labeled_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator))
        states.push_back(successor.node.get_state());

    using Context = kr::dl::semantics::StateEvaluationContext<Ext, Kind>;
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto empty_arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto register_data = ygg::Data<kr::dl::semantics::RegisterValues>();
    register_data.concept_values.resize(1);
    register_data.role_values.resize(1);
    const auto arguments = kr::dl::semantics::insert(denotations, empty_arguments).first;
    const auto registers = kr::dl::semantics::insert(denotations, register_data).first;
    // One storage per state, so dynamic results are never shared across states.
    auto storages = std::vector<std::unique_ptr<kr::dl::semantics::EvaluationStorage<Ext>>>();
    auto contexts = std::vector<std::unique_ptr<Context>>();
    for (const auto& state : states)
    {
        storages.push_back(std::make_unique<kr::dl::semantics::EvaluationStorage<Ext>>(denotations));
        contexts.push_back(std::make_unique<Context>(state, builder, *storages.back(), arguments, registers));
    }
    auto context_pointers = std::vector<Context*>();
    for (const auto& context : contexts)
        context_pointers.push_back(context.get());

    const auto triple = std::string(R"((q_atomic_state "triple" (x y z)))");
    const auto fixed = std::string(R"((q_atomic_state "fixed" (x y z)))");
    const auto edges = std::string(R"((r_or (r_atomic_state "edge") (r_inverse (r_atomic_state "edge"))))");
    const auto triangle = "(n_count (q_join (q_join (q_role (x y) " + edges + ") (q_role (y z) " + edges + ")) (q_role (x z) " + edges + ")))";
    const auto expressions = std::vector<std::string> {
        triangle,
        R"((n_count (q_project (x) (q_join (q_atomic_state "assignment" (x y z w)) (q_atomic_state "required" (x y z))))))",
        "(n_count (q_join " + triple + R"( (q_role (x y) (r_atomic_state "edge")))))",
        "(n_count (q_select_value x \"a\" (q_join " + triple + " " + fixed + ")))",
        "(n_count (q_union " + triple + " " + fixed + "))",
        // A query nested inside a leaf's concept argument is optimized too.
        "(n_count (c_project x (q_join " + triple + " (q_concept x (c_project z (q_join " + triple + " " + fixed + "))))))",
        "(n_count (q_generic_join (x y z) (x y z) " + triple + " " + fixed + "))",
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
        for (const auto* mode : { &statistics, static_cast<const kr::dl::QueryStatistics<Ext>*>(nullptr) })
        {
            const auto optimized = mode ? kr::dl::optimize(original, *repository, *mode) : kr::dl::optimize(original, *repository);
            const auto formatted = fmt::format("{}", optimized);
            EXPECT_EQ(kr::ps::ext::dl::parse_numerical(formatted, domain, *repository), optimized) << formatted;
            for (auto* context : context_pointers)
                EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(optimized, *context).get(), kr::dl::semantics::evaluate<Kind>(original, *context).get())
                    << text << " => " << formatted;
            if (text == triangle)
                EXPECT_NE(formatted.find("q_generic_join"), std::string::npos) << formatted;
        }
    }
}

}  // namespace

TEST(RunirQueryOptimization, GroundOptimizedFeaturesEvaluateLikeOriginals) { check_optimized_queries<tyr::GroundTag>(); }
TEST(RunirQueryOptimization, LiftedOptimizedFeaturesEvaluateLikeOriginals) { check_optimized_queries<tyr::LiftedTag>(); }

}  // namespace runir::tests
