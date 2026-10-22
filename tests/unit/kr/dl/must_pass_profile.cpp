// Profiles the full evaluation of the first q_must_pass query of a program file in the initial state of a lifted
// task, phase by phase, with yggdrasil's search statistics. Not a test.
//
//   runir_must_pass_profile <domain.pddl> <task.pddl> <program file> [repeats]

#include "planning_fixtures.hpp"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/detail/evaluation_helpers.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/distance_evaluation.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/state_evaluation.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <string>
#include <tyr/planning/lifted/successor_generator.hpp>

namespace
{
using Kind = tyr::LiftedTag;

double seconds_since(std::chrono::steady_clock::time_point start)
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
}

/// The first balanced "(q_must_pass ...)" of a text.
std::string must_pass_text(const std::string& text)
{
    const auto start = text.find("(q_must_pass");
    auto depth = 0;
    for (auto i = start; i < text.size(); ++i)
    {
        depth += text[i] == '(' ? 1 : text[i] == ')' ? -1 : 0;
        if (depth == 0)
            return text.substr(start, i - start + 1);
    }
    throw std::invalid_argument("no q_must_pass query");
}
}  // namespace

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        std::fprintf(stderr, "usage: %s <domain.pddl> <task.pddl> <program> [repeats]\n", argv[0]);
        return 2;
    }
    const auto repeats = argc > 4 ? std::stoul(argv[4]) : 3ul;
    auto file = std::ifstream(argv[3]);
    const auto text = must_pass_text(std::string(std::istreambuf_iterator<char>(file), {}));
    const auto search = runir::tests::make_lifted_context(argv[1], argv[2]);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = runir::kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    const auto count = runir::kr::ps::ext::dl::parse_numerical("(n_count " + text + ")", domain, *repository);
    const auto query = count.get_variant()
                           .template get<ygg::Index<runir::kr::dl::Numerical<runir::kr::dl::CountTag>>>()
                           .get_arg()
                           .template get<ygg::Index<runir::kr::dl::Query<>>>();
    const auto must_pass = query.get_variant().template get<ygg::Index<runir::kr::dl::Query<runir::kr::dl::QueryMustPassTag>>>();

    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    auto builder = runir::kr::dl::semantics::Builder();
    auto denotations = runir::kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto empty_arguments = ygg::Data<runir::kr::dl::semantics::CallArguments>();
    auto register_data = ygg::Data<runir::kr::dl::semantics::RegisterValues>();
    register_data.concept_values.resize(1);
    register_data.role_values.resize(1);
    const auto arguments = runir::kr::dl::semantics::insert(denotations, empty_arguments).first;
    const auto registers = runir::kr::dl::semantics::insert(denotations, register_data).first;

    auto feature = std::optional<runir::kr::dl::semantics::MustPassFeature>();
    for (size_t repeat = 0; repeat < repeats; ++repeat)
    {
        auto storage = runir::kr::dl::semantics::EvaluationStorage(denotations);
        auto context = runir::kr::dl::semantics::StateEvaluationContext<Kind>(initial.get_state(), builder, storage, arguments, registers);
        auto start = std::chrono::steady_clock::now();
        const auto sources = runir::kr::dl::semantics::evaluate<Kind>(must_pass.get_sources(), context);
        const auto targets = runir::kr::dl::semantics::evaluate<Kind>(must_pass.get_targets(), context);
        const auto probes = runir::kr::dl::semantics::evaluate<Kind>(must_pass.get_probes(), context);
        const auto arguments_time = seconds_since(start);
        start = std::chrono::steady_clock::now();
        if (!feature)
            feature.emplace(runir::kr::dl::semantics::detail::make_must_pass_feature(must_pass.get_data().plan, must_pass.get_edges()));
        const auto translate_time = seconds_since(start);
        const auto edges = must_pass.get_edges();
        auto slot_time = 0.0;
        const auto input = [&](size_t slot)
        {
            const auto begin = std::chrono::steady_clock::now();
            const auto rows = runir::kr::dl::semantics::detail::evaluate_slot<Kind>(feature->slots[slot], edges.get_context(), context);
            slot_time += seconds_since(begin);
            return rows;
        };
        start = std::chrono::steady_clock::now();
        auto& heuristic =
            runir::kr::dl::semantics::detail::must_pass_heuristic(*feature, runir::kr::dl::semantics::detail::num_objects<Kind>(context), input, targets);
        const auto heuristic_time = seconds_since(start);
        auto out = ygg::Builder<ygg::database::Relation>(must_pass.get_schema().span());
        const auto before = feature->must_pass.statistics();
        start = std::chrono::steady_clock::now();
        feature->must_pass.must_pass(input, sources, targets, probes, out, heuristic);
        const auto must_pass_time = seconds_since(start);
        const auto& after = feature->must_pass.statistics();
        std::printf("repeat %zu: arguments %.3f s (targets %zu rows), translate %.3f s, heuristic %.3f s, must_pass %.3f s (slots %.3f s): rows %zu, "
                    "keys %zu, expanded %zu, backward %zu, edges %zu, removals %zu\n",
                    repeat,
                    arguments_time,
                    targets.size(),
                    translate_time,
                    heuristic_time,
                    must_pass_time,
                    slot_time,
                    out.size(),
                    after.keys - before.keys,
                    after.vertices - before.vertices,
                    after.backward - before.backward,
                    after.edges - before.edges,
                    after.removals - before.removals);
    }
    return 0;
}
