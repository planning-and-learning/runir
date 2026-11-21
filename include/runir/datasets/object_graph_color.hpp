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

#ifndef RUNIR_DATASETS_OBJECT_GRAPH_COLOR_HPP_
#define RUNIR_DATASETS_OBJECT_GRAPH_COLOR_HPP_

#include <algorithm>
#include <boost/mp11.hpp>
#include <cassert>
#include <cista/containers/variant.h>
#include <cista/containers/vector.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/formalism/predicate_data.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/config.hpp>
#include <urd/elements/concepts.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/formalism/declarations.hpp>
#include <yggdrasil/formalism/nested.hpp>
#include <yggdrasil/formalism/symbol_context.hpp>
#include <yggdrasil/semantics/canonicalization.hpp>

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

namespace urd
{

template<::tyr::formalism::FactKind T>
struct data<runir::datasets::PredicateColor<T>>
{
    urd::index<runir::datasets::PredicateColor<T>> index;
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
};

template<>
struct data<runir::datasets::Color>
{
    urd::index<runir::datasets::Color> index;
    urd::flat_vector<runir::datasets::ColorVariant> values;

    data() = default;

    explicit data(urd::flat_vector<runir::datasets::ColorVariant> values_) : index(), values(std::move(values_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"values", &data::values>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::values> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::values> {}; }
};

}  // namespace urd

namespace urd
{
template<::tyr::formalism::FactKind T, typename V>
struct view_mixin<runir::datasets::PredicateColor<T>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_predicate() const noexcept { return self().template member<&urd::record_of<V>::predicate>(); }

    auto get_argument_position() const noexcept { return self().template member<&urd::record_of<V>::argument_position>(); }

    auto get_context() const noexcept { return self().template member<&urd::record_of<V>::context>(); }
};

template<typename V>
struct view_mixin<runir::datasets::Color, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_values() const noexcept { return self().template member<&urd::record_of<V>::values>(); }
};
}  // namespace urd

namespace runir::datasets
{

template<::tyr::formalism::FactKind T>
bool is_canonical(const urd::data<PredicateColor<T>>&) noexcept
{ return true; }

template<::tyr::formalism::FactKind T>
void canonicalize(urd::data<PredicateColor<T>>&) noexcept
{
}

inline bool is_canonical(const urd::data<Color>& data) noexcept { return ygg::is_canonical(data.values); }

inline void canonicalize(urd::data<Color>& data) { ygg::canonicalize(data.values); }

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
    using NestsBase::repository_for;
    using NestsBase::view;

    template<typename T>
    std::optional<urd::index_view<T, ColorRepository>> find(const urd::data<T>& data) const noexcept
    {
        assert(is_canonical(data));
        return Base::find(data);
    }

    template<typename T>
    std::pair<urd::index_view<T, ColorRepository>, bool> insert(urd::data<T>& data)
    {
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
