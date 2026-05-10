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

#ifndef RUNIR_DATASETS_CLASS_OF_TASKS_HPP_
#define RUNIR_DATASETS_CLASS_OF_TASKS_HPP_

#include <tuple>
#include <tyr/planning/planning.hpp>
#include <utility>
#include <vector>

namespace runir::datasets
{

template<tyr::planning::TaskKind Kind>
struct TaskClass
{
    std::vector<tyr::planning::TaskPtr<Kind>> tasks;

    auto identifying_members() const noexcept { return std::tie(tasks); }
};

template<tyr::planning::TaskKind Kind>
struct TaskSearchContext
{
    tyr::planning::TaskPtr<Kind> task;
    tyr::ExecutionContextPtr execution_context;
    tyr::planning::SuccessorGeneratorPtr<Kind> successor_generator;
    tyr::planning::StateRepositoryPtr<Kind> state_repository;
    tyr::planning::AxiomEvaluatorPtr<Kind> axiom_evaluator;

    TaskSearchContext() = default;

    TaskSearchContext(tyr::planning::TaskPtr<Kind> task_, tyr::ExecutionContextPtr execution_context_) :
        task(std::move(task_)),
        execution_context(std::move(execution_context_)),
        successor_generator(tyr::planning::SuccessorGenerator<Kind>::create(task, execution_context)),
        state_repository(successor_generator->get_state_repository()),
        axiom_evaluator(state_repository->get_axiom_evaluator())
    {
    }
};

template<tyr::planning::TaskKind Kind>
struct TaskClassSearchContexts
{
    std::vector<TaskSearchContext<Kind>> contexts;

    TaskClassSearchContexts() = default;

    explicit TaskClassSearchContexts(std::vector<TaskSearchContext<Kind>> contexts_) : contexts(std::move(contexts_)) {}

    TaskClassSearchContexts(const TaskClass<Kind>& task_class, tyr::ExecutionContextPtr execution_context)
    {
        contexts.reserve(task_class.tasks.size());
        for (const auto& task : task_class.tasks)
            contexts.emplace_back(task, execution_context);
    }
};

}  // namespace runir::datasets

#endif
