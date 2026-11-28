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

#ifndef RUNIR_DATASETS_OBJECT_GRAPH_PREDICATE_COLOR_VIEW_HPP_
#define RUNIR_DATASETS_OBJECT_GRAPH_PREDICATE_COLOR_VIEW_HPP_

#include "runir/datasets/object_graph/predicate_color_data.hpp"

#include <tyr/formalism/predicate_view.hpp>
#include <urd/elements/view.hpp>

namespace urd
{
template<::tyr::formalism::FactKind T, typename V>
struct view_mixin<runir::datasets::PredicateColor<T>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto predicate() const noexcept { return self().template member<&urd::record_of<V>::predicate>(); }

    auto argument_position() const noexcept { return self().template member<&urd::record_of<V>::argument_position>(); }

    auto predicate_context() const noexcept { return self().template member<&urd::record_of<V>::predicate_context>(); }
};
}  // namespace urd

#endif
