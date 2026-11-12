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
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/formalism/predicate_data.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/config.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/formalism/declarations.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>
#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::datasets
{

template<::tyr::formalism::FactKind T>
struct PredicateColor
{
};

struct Color
{
};

enum class PredicateContext : std::uint8_t
{
    STATE,
    GOAL,
};

class ColorRepository;
class ColorRepositoryFactory;

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
    data(const data&) = default;
    data& operator=(const data&) = default;
    data(data&&) = default;
    data& operator=(data&&) = default;
    ~data() = default;

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::predicate, &data::argument_position, &data::context> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::predicate, &data::argument_position, &data::context> {}; }
};

template<>
struct data<runir::datasets::Color>
{
    using Variant = urd::index_variant<
        boost::mp11::mp_transform<runir::datasets::PredicateColor, boost::mp11::mp_list<::tyr::formalism::StaticTag, ::tyr::formalism::FluentTag>>>;
    using VariantList = urd::flat_vector<Variant>;

    urd::index<runir::datasets::Color> index;
    VariantList values;

    data() = default;
    explicit data(VariantList values_) : index(), values(std::move(values_)) {}
    data(const data&) = delete;
    data& operator=(const data&) = delete;
    data(data&&) = default;
    data& operator=(data&&) = default;
    ~data() = default;

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::values> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::values> {}; }
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
    auto get_predicate() const noexcept { return urd::resolve(self().data().predicate, self().context().get_planning_repository()); }
    auto get_argument_position() const noexcept { return self().data().argument_position; }
    auto get_predicate_context() const noexcept { return self().data().context; }
};

template<typename V>
struct view_mixin<runir::datasets::Color, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_colors() const noexcept { return urd::resolve(self().data().values, self().context()); }
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

class ColorRepository
{
    friend class ColorRepositoryFactory;

private:
    ygg::formalism::ConcurrentSymbolRepository<PredicateColor<::tyr::formalism::StaticTag>, PredicateColor<::tyr::formalism::FluentTag>, Color>
        m_symbol_repository;
    std::shared_ptr<const ::tyr::formalism::planning::Repository> m_planning_repository;
    std::size_t m_index;

    ColorRepository(std::size_t index, std::shared_ptr<const ::tyr::formalism::planning::Repository> planning_repository) :
        m_symbol_repository(nullptr),
        m_planning_repository(std::move(planning_repository)),
        m_index(index)
    { assert(m_planning_repository); }

public:
    ColorRepository(const ColorRepository&) = delete;
    ColorRepository& operator=(const ColorRepository&) = delete;
    ColorRepository(ColorRepository&&) = delete;
    ColorRepository& operator=(ColorRepository&&) = delete;
    ~ColorRepository() = default;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }
    [[nodiscard]] const auto& get_planning_repository() const noexcept
    {
        assert(m_planning_repository);
        return *m_planning_repository;
    }
    [[nodiscard]] const auto& get_planning_repository_ptr() const noexcept { return m_planning_repository; }

    void clear() noexcept { m_symbol_repository.clear(); }

    template<typename T>
    std::optional<urd::index_view<T, ColorRepository>> find(const urd::data<T>& data) const noexcept
    {
        assert(is_canonical(data));
        if (const auto index = m_symbol_repository.get<T>().find(data); !index.is_none())
            return urd::index_view<T, ColorRepository>(index, this);
        return std::nullopt;
    }

    template<typename T>
    std::pair<urd::index_view<T, ColorRepository>, bool> insert(urd::data<T>& data)
    {
        canonicalize(data);
        assert(is_canonical(data));
        const auto [index, created] = ygg::formalism::intern(m_symbol_repository.get<T>(), data);
        return { urd::index_view<T, ColorRepository>(index, this), created };
    }

    /// An index as its view here (urd's resolves contract): what compositions of indices are read through (urd::resolve).
    template<typename T>
    [[nodiscard]] urd::index_view<T, ColorRepository> view(urd::index<T> index) const noexcept
    { return { index, this }; }

    template<typename T>
    const urd::data<T>& operator[](urd::index<T> index) const noexcept
    { return m_symbol_repository[index]; }

    template<typename T>
    [[nodiscard]] std::size_t size() const noexcept
    { return m_symbol_repository.size<T>(); }
};

class ColorRepositoryFactory
{
private:
    std::size_t m_next_index = 0;

public:
    std::shared_ptr<ColorRepository> create(std::shared_ptr<const ::tyr::formalism::planning::Repository> planning_repository)
    { return std::shared_ptr<ColorRepository>(new ColorRepository(m_next_index++, std::move(planning_repository))); }
};

using ColorView = urd::index_view<Color, ColorRepository>;

inline const ColorRepository& get_repository(const ColorRepository& repository URD_LIFETIMEBOUND) noexcept { return repository; }
inline ColorRepository& get_repository(ColorRepository& repository URD_LIFETIMEBOUND) noexcept { return repository; }

}  // namespace runir::datasets

#endif
