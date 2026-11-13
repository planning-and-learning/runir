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

#ifndef RUNIR_DATASETS_EQUIVALENCE_POLICIES_GI_HPP_
#define RUNIR_DATASETS_EQUIVALENCE_POLICIES_GI_HPP_

#include "runir/datasets/equivalence_policy.hpp"
#include "runir/datasets/object_graph.hpp"

#include <concepts>
#include <mutex>
#include <vefr/nauty.hpp>

namespace runir::datasets
{

struct GIEquivalenceTag
{
};

template<>
struct EquivalencePolicy<GIEquivalenceTag>
{
private:
    ColorRepository& m_colors;
    urd::Store<urd::map<vefr::CanonicalForm, StateGraphVertexRef>> m_certificate_to_representative;
    std::mutex m_mutex;

public:
    explicit EquivalencePolicy(ColorRepository& colors) : m_colors(colors) {}

    template<tyr::TaskKind Kind, std::invocable Allocate>
    StateGraphVertexRef get_or_create_representative(const StateGraphVertexCandidate<Kind>& candidate, Allocate allocate)
    {
        auto object_graph = create_object_graph(candidate.state, m_colors);
        auto certificate = vefr::canonical_form(*object_graph, [&](VertexIndex v) { return object_graph->vertex_value(v).index(); });

        const auto lock = std::lock_guard(m_mutex);
        if (const auto it = m_certificate_to_representative.find(certificate); it != m_certificate_to_representative.end())
            return it->second;

        const auto representative = allocate();
        m_certificate_to_representative.emplace(std::move(certificate), representative);
        return representative;
    }
};

}  // namespace runir::datasets

#endif
