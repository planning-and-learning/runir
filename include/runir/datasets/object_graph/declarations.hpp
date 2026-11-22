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


#ifndef RUNIR_DATASETS_OBJECT_GRAPH_DECLARATIONS_HPP_
#define RUNIR_DATASETS_OBJECT_GRAPH_DECLARATIONS_HPP_

#include <cista/containers/variant.h>
#include <cstdint>
#include <string_view>
#include <tyr/formalism/declarations.hpp>
#include <urd/elements/index.hpp>

namespace runir::datasets
{

template<::tyr::formalism::FactKind T>
struct PredicateColor
{
    static constexpr std::string_view name = urd::joined_name<"PredicateColor", T::name>;
};

struct Color
{
    static constexpr auto name = "Color";
};

enum class PredicateContext : std::uint8_t
{
    STATE,
    GOAL,
};

class ColorRepository;

/// One predicate color of a vertex color: of a static or a fluent predicate.
using ColorVariant =
    urd::index_variant<boost::mp11::mp_transform<PredicateColor, boost::mp11::mp_list<::tyr::formalism::StaticTag, ::tyr::formalism::FluentTag>>>;

}  // namespace runir::datasets

#endif
