/*
 * Copyright (C) 2025-2026 Dominik Drexler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <argparse/argparse.hpp>
#include <fmt/format.h>
#include <iostream>
#include <runir/runir.hpp>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/planning.hpp>

namespace
{

template<tyr::planning::TaskKind Kind>
void print_state_graph(tyr::planning::TaskPtr<Kind> task, tyr::ExecutionContextPtr execution_context)
{
    auto context = runir::datasets::TaskSearchContext<Kind>(std::move(task), std::move(execution_context));
    const auto graph = runir::datasets::generate_state_graph(context);
    fmt::print("{}", graph);
}

}  // namespace

int main(int argc, char** argv)
{
    auto program = argparse::ArgumentParser("Generate a planning task state graph in Graphviz DOT format.");
    program.add_argument("-D", "--domain-filepath").required().help("The path to the PDDL domain file.");
    program.add_argument("-P", "--problem-filepath").required().help("The path to the PDDL problem file.");
    program.add_argument("-N", "--num-worker-threads").default_value(std::size_t(1)).scan<'u', std::size_t>().help("The number of worker threads.");
    program.add_argument("-G", "--instantiate-ground-task")
        .default_value(false)
        .implicit_value(true)
        .help("Instantiate the task first and generate the ground state graph.");
    program.add_argument("--disable-invariant-synthesis")
        .default_value(false)
        .implicit_value(true)
        .help("Disable invariant synthesis during ground task instantiation.");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what() << "\n";
        std::cerr << program;
        return 1;
    }

    const auto domain_filepath = program.get<std::string>("--domain-filepath");
    const auto problem_filepath = program.get<std::string>("--problem-filepath");
    const auto num_worker_threads = program.get<std::size_t>("--num-worker-threads");
    const auto instantiate_ground_task = program.get<bool>("--instantiate-ground-task");
    const auto disable_invariant_synthesis = program.get<bool>("--disable-invariant-synthesis");

    auto parser_options = loki::ParserOptions();
    auto parser = tyr::formalism::planning::Parser(domain_filepath, parser_options);
    auto lifted_task = tyr::planning::Task<tyr::planning::LiftedTag>::create(parser.parse_task(problem_filepath));
    auto execution_context = tyr::ExecutionContext::create(num_worker_threads);

    if (!instantiate_ground_task)
    {
        print_state_graph(lifted_task, execution_context);
        return 0;
    }

    auto options = tyr::planning::GroundTaskInstantiationOptions();
    options.disable_invariant_synthesis = disable_invariant_synthesis;
    auto result = lifted_task->instantiate_ground_task(*execution_context, options);

    if (result.status == tyr::planning::GroundTaskInstantiationStatus::PROVEN_UNSOLVABLE)
    {
        std::cerr << "Task was proven unsolvable during ground task instantiation.\n";
        return 2;
    }

    print_state_graph(result.task, execution_context);
    return 0;
}
