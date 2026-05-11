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

#ifndef RUNIR_DATASETS_TASK_CLASS_HPP_
#define RUNIR_DATASETS_TASK_CLASS_HPP_

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
    tyr::planning::AxiomEvaluatorFactory<Kind> axiom_evaluator_factory;
    tyr::planning::StateRepositoryFactory<Kind> state_repository_factory;
    tyr::planning::SuccessorGeneratorFactory<Kind> successor_generator_factory;
    tyr::planning::AxiomEvaluatorPtr<Kind> axiom_evaluator;
    tyr::planning::StateRepositoryPtr<Kind> state_repository;
    tyr::planning::SuccessorGeneratorPtr<Kind> successor_generator;

    TaskSearchContext() = default;

    TaskSearchContext(tyr::planning::TaskPtr<Kind> task_, tyr::ExecutionContextPtr execution_context_) :
        task(std::move(task_)),
        execution_context(std::move(execution_context_)),
        axiom_evaluator_factory(),
        state_repository_factory(),
        successor_generator_factory(),
        axiom_evaluator(axiom_evaluator_factory.create(task, execution_context)),
        state_repository(state_repository_factory.create(task, axiom_evaluator)),
        successor_generator(successor_generator_factory.create(task, execution_context, state_repository))
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
