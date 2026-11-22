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


#ifndef RUNIR_DATASETS_OBJECT_GRAPH_REPOSITORY_HPP_
#define RUNIR_DATASETS_OBJECT_GRAPH_REPOSITORY_HPP_

#include "runir/datasets/object_graph/canonicalization.hpp"
#include "runir/datasets/object_graph/color_data.hpp"
#include "runir/datasets/object_graph/color_view.hpp"
#include "runir/datasets/object_graph/declarations.hpp"
#include "runir/datasets/object_graph/predicate_color_data.hpp"
#include "runir/datasets/object_graph/predicate_color_view.hpp"

#include <boost/mp11.hpp>
#include <cassert>
#include <cstddef>
#include <memory>
#include <optional>
#include <tyr/formalism/repository.hpp>
#include <urd/config.hpp>
#include <utility>
#include <yggdrasil/formalism/declarations.hpp>
#include <yggdrasil/formalism/nested.hpp>
#include <yggdrasil/formalism/symbol_context.hpp>

namespace runir::datasets
{
/// The symbol types of a color repository.
using ColorTypes = boost::mp11::mp_list<PredicateColor<::tyr::formalism::StaticTag>, PredicateColor<::tyr::formalism::FluentTag>, Color>;

/// Colors, over the planning repository whose predicates they refer to (nested, read only).
class ColorRepository :
    public ygg::formalism::SymbolContext<ColorRepository, ColorTypes, true>,
    public ygg::formalism::Nests<ColorRepository, ygg::formalism::Nested<const ::tyr::formalism::Repository>>
{
    using Base = ygg::formalism::SymbolContext<ColorRepository, ColorTypes, true>;
    using NestsBase = ygg::formalism::Nests<ColorRepository, ygg::formalism::Nested<const ::tyr::formalism::Repository>>;

private:
    std::size_t m_index;

public:
    /// Over the planning repository whose predicates its colors refer to. Numbered by ygg::next_layer(), the process-wide
    /// counter, so that no two contexts share a number.
    explicit ColorRepository(std::shared_ptr<const ::tyr::formalism::Repository> planning_repository) :
        Base(),
        NestsBase(std::move(planning_repository)),
        m_index(ygg::next_layer())
    {
    }

    ColorRepository(const ColorRepository&) = delete;
    ColorRepository& operator=(const ColorRepository&) = delete;
    ColorRepository(ColorRepository&&) = delete;
    ColorRepository& operator=(ColorRepository&&) = delete;
    ~ColorRepository() = default;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }

    using Base::view;
    using NestsBase::get;
    using NestsBase::get_ptr;

    template<typename T>
    std::optional<urd::index_view<T, ColorRepository>> find(const urd::data<T>& data) const noexcept
    {
        using ygg::formalism::is_canonical;  // the default, for a kind without a canonical form of its own
        assert(is_canonical(data));
        return Base::find(data);
    }

    template<typename T>
    std::pair<urd::index_view<T, ColorRepository>, bool> insert(urd::data<T>& data)
    {
        using ygg::formalism::canonicalize;  // the default, for a kind without a canonical form of its own
        using ygg::formalism::is_canonical;
        canonicalize(data);
        assert(is_canonical(data));
        return Base::insert(data);
    }
};

using ColorView = urd::index_view<Color, ColorRepository>;

inline const ColorRepository& get_repository(const ColorRepository& repository URD_LIFETIMEBOUND) noexcept { return repository; }

inline ColorRepository& get_repository(ColorRepository& repository URD_LIFETIMEBOUND) noexcept { return repository; }

}  // namespace runir::datasets

#endif
