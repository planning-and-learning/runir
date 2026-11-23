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

#ifndef RUNIR_DATASETS_OBJECT_GRAPH_PREDICATE_COLOR_DATA_HPP_
#define RUNIR_DATASETS_OBJECT_GRAPH_PREDICATE_COLOR_DATA_HPP_

#include "runir/datasets/object_graph/declarations.hpp"

#include <tyr/formalism/predicate_data.hpp>
#include <urd/elements/concepts.hpp>
#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

template<::tyr::formalism::FactKind T>
struct data<runir::datasets::PredicateColor<T>>
{
    urd::key<runir::datasets::PredicateColor<T>> index;
    urd::index<::tyr::formalism::Predicate<T>> predicate;
    ygg::uint_t argument_position = 0;
    runir::datasets::PredicateContext context {};

    data() = default;

    data(urd::index<::tyr::formalism::Predicate<T>> predicate_, ygg::uint_t argument_position_, runir::datasets::PredicateContext context_) noexcept :
        index(),
        predicate(predicate_),
        argument_position(argument_position_),
        context(context_)
    {
    }

    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"predicate", &data::predicate>,
                           urd::field<"argument_position", &data::argument_position>,
                           urd::field<"context", &data::context>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::predicate, &data::argument_position, &data::context> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::predicate, &data::argument_position, &data::context> {}; }

    [[nodiscard]] urd::key<runir::datasets::PredicateColor<T>> key() const noexcept { return urd::key<runir::datasets::PredicateColor<T>>(index); }
};

}  // namespace urd

#endif
