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


#ifndef RUNIR_DATASETS_OBJECT_GRAPH_CANONICALIZATION_HPP_
#define RUNIR_DATASETS_OBJECT_GRAPH_CANONICALIZATION_HPP_

#include "runir/datasets/object_graph/color_data.hpp"

#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::datasets
{

inline bool is_canonical(const urd::data<Color>& data) noexcept { return ygg::is_canonical(data.values); }

inline void canonicalize(urd::data<Color>& data) { ygg::canonicalize(data.values); }
}  // namespace runir::datasets

#endif
