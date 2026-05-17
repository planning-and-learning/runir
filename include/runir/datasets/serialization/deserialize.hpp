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

#ifndef RUNIR_DATASETS_SERIALIZATION_DESERIALIZE_HPP_
#define RUNIR_DATASETS_SERIALIZATION_DESERIALIZE_HPP_

#include "runir/datasets/serialization/context.hpp"
#include "runir/datasets/task_class.hpp"

#include <concepts>
#include <filesystem>
#include <type_traits>

#include <tyr/formalism/planning/parser.hpp>

namespace runir::datasets::serialization
{

template<tyr::planning::TaskKind Kind>
auto deserialize(DomainDeserializationContext context, const StateGraphArchive& archive) -> TaskDeserializationContext<Kind>
{
    namespace p = tyr::planning;

    auto planning_task = context.parser->parse_task(archive.problem, std::filesystem::path("<serialized-problem>"));

    if constexpr (std::same_as<Kind, p::GroundTag>)
    {
        auto lifted_task = p::Task<p::LiftedTag>(planning_task);
        auto task = lifted_task.instantiate_ground_task(*context.execution_context).task;
        return TaskDeserializationContext<p::GroundTag> { TaskSearchContext<p::GroundTag>(std::move(task), std::move(context.execution_context)) };
    }
    else if constexpr (std::same_as<Kind, p::LiftedTag>)
    {
        auto task = p::Task<p::LiftedTag>::create(std::move(planning_task));
        return TaskDeserializationContext<p::LiftedTag> { TaskSearchContext<p::LiftedTag>(std::move(task), std::move(context.execution_context)) };
    }
}

template<tyr::planning::TaskKind Kind>
auto deserialize(tyr::formalism::planning::Parser& parser, const StateGraphArchive& archive, tyr::ExecutionContextPtr execution_context) -> TaskSearchContext<Kind>
{
    auto context = deserialize<Kind>(DomainDeserializationContext { &parser, std::move(execution_context) }, archive);
    return std::move(context.search_context);
}

}  // namespace runir::datasets::serialization

#endif
