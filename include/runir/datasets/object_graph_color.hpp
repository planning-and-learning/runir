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
#include <utility>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>
#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/semantics/comparison.hpp>

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

namespace ygg
{

template<::tyr::formalism::FactKind T>
struct Index<runir::datasets::PredicateColor<T>> : IndexMixin<Index<runir::datasets::PredicateColor<T>>>
{
    using Base = IndexMixin<Index<runir::datasets::PredicateColor<T>>>;
    using Base::Base;
};

template<>
struct Index<runir::datasets::Color> : IndexMixin<Index<runir::datasets::Color>>
{
    using Base = IndexMixin<Index<runir::datasets::Color>>;
    using Base::Base;
};

template<::tyr::formalism::FactKind T>
struct Data<runir::datasets::PredicateColor<T>>
{
    Index<runir::datasets::PredicateColor<T>> index;
    Index<::tyr::formalism::Predicate<T>> predicate;
    uint_t argument_position = 0;
    runir::datasets::PredicateContext context {};

    Data() = default;
    Data(Index<::tyr::formalism::Predicate<T>> predicate_, uint_t argument_position_, runir::datasets::PredicateContext context_) noexcept :
        index(),
        predicate(predicate_),
        argument_position(argument_position_),
        context(context_)
    {
    }
    Data(const Data&) = default;
    Data& operator=(const Data&) = default;
    Data(Data&&) = default;
    Data& operator=(Data&&) = default;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(predicate);
        ygg::clear(argument_position);
        ygg::clear(context);
    }

    auto cista_members() const noexcept { return std::tie(index, predicate, argument_position, context); }
    auto identifying_members() const noexcept { return std::tie(predicate, argument_position, context); }
};

template<>
struct Data<runir::datasets::Color>
{
    using Variant = cista::variant<Index<runir::datasets::PredicateColor<::tyr::formalism::StaticTag>>,
                                   Index<runir::datasets::PredicateColor<::tyr::formalism::FluentTag>>>;
    using VariantList = cista::offset::vector<Variant>;

    Index<runir::datasets::Color> index;
    VariantList values;

    Data() = default;
    explicit Data(VariantList values_) : index(), values(std::move(values_)) {}
    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;
    Data(Data&&) = default;
    Data& operator=(Data&&) = default;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(values);
    }

    auto cista_members() const noexcept { return std::tie(index, values); }
    auto identifying_members() const noexcept { return std::tie(values); }
};

template<::tyr::formalism::FactKind T, typename C>
class View<Index<runir::datasets::PredicateColor<T>>, C>
{
private:
    const C* m_context;
    Index<runir::datasets::PredicateColor<T>> m_handle;

public:
    View(Index<runir::datasets::PredicateColor<T>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return (*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_predicate() const noexcept { return ygg::make_view(get_data().predicate, m_context->get_planning_repository()); }
    auto get_argument_position() const noexcept { return get_data().argument_position; }
    auto get_predicate_context() const noexcept { return get_data().context; }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

template<typename C>
class View<Index<runir::datasets::Color>, C>
{
private:
    Index<runir::datasets::Color> m_handle;
    const C* m_context;

public:
    View(Index<runir::datasets::Color> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const noexcept { return (*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_colors() const noexcept { return ygg::make_view(get_data().values, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

static_assert(ygg::uses_trivial_storage_v<runir::datasets::PredicateColor<::tyr::formalism::StaticTag>>);
static_assert(ygg::uses_trivial_storage_v<runir::datasets::PredicateColor<::tyr::formalism::FluentTag>>);
static_assert(!ygg::uses_trivial_storage_v<runir::datasets::Color>);

}  // namespace ygg

namespace runir::datasets
{

template<::tyr::formalism::FactKind T>
bool is_canonical(const ygg::Data<PredicateColor<T>>&) noexcept
{
    return true;
}

template<::tyr::formalism::FactKind T>
void canonicalize(ygg::Data<PredicateColor<T>>&) noexcept
{
}

inline bool is_canonical(const ygg::Data<Color>& data) noexcept
{
    return std::is_sorted(data.values.begin(), data.values.end()) && std::adjacent_find(data.values.begin(), data.values.end()) == data.values.end();
}

inline void canonicalize(ygg::Data<Color>& data)
{
    std::sort(data.values.begin(), data.values.end());
    data.values.erase(std::unique(data.values.begin(), data.values.end()), data.values.end());
}

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
    {
        assert(m_planning_repository);
    }

public:
    ColorRepository(const ColorRepository&) = delete;
    ColorRepository& operator=(const ColorRepository&) = delete;
    ColorRepository(ColorRepository&&) = delete;
    ColorRepository& operator=(ColorRepository&&) = delete;

    const auto& get_index() const noexcept { return m_index; }
    const auto& get_planning_repository() const noexcept
    {
        assert(m_planning_repository);
        return *m_planning_repository;
    }
    const auto& get_planning_repository_ptr() const noexcept { return m_planning_repository; }

    void clear() noexcept { m_symbol_repository.clear(); }

    template<typename T>
    std::optional<ygg::View<ygg::Index<T>, ColorRepository>> find(const ygg::Data<T>& data) const noexcept
    {
        assert(is_canonical(data));
        if (const auto index = m_symbol_repository.find_local<T>(data))
            return ygg::View<ygg::Index<T>, ColorRepository>(*index, *this);
        return std::nullopt;
    }

    template<typename T>
    std::pair<ygg::View<ygg::Index<T>, ColorRepository>, bool> get_or_create(ygg::Data<T>& data)
    {
        canonicalize(data);
        assert(is_canonical(data));
        const auto [index, created] = m_symbol_repository.get_or_create_local<T>(data);
        return { ygg::View<ygg::Index<T>, ColorRepository>(index, *this), created };
    }

    template<typename T>
    const ygg::Data<T>& operator[](ygg::Index<T> index) const noexcept
    {
        assert(m_symbol_repository.is_local<T>(index));
        return m_symbol_repository.at_local<T>(index);
    }

    template<typename T>
    std::size_t size() const noexcept
    {
        return m_symbol_repository.local_size<T>();
    }

    template<typename T>
    const ColorRepository& get_canonical_context(ygg::Index<T>) const noexcept
    {
        return *this;
    }
};

class ColorRepositoryFactory
{
private:
    std::size_t m_next_index = 0;

public:
    std::shared_ptr<ColorRepository> create(std::shared_ptr<const ::tyr::formalism::planning::Repository> planning_repository)
    {
        return std::shared_ptr<ColorRepository>(new ColorRepository(m_next_index++, std::move(planning_repository)));
    }
};

using ColorView = ygg::View<ygg::Index<Color>, ColorRepository>;

inline const ColorRepository& get_repository(const ColorRepository& repository) noexcept { return repository; }
inline ColorRepository& get_repository(ColorRepository& repository) noexcept { return repository; }

}  // namespace runir::datasets

#endif
