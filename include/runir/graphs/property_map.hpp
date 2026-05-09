#ifndef RUNIR_GRAPHS_PROPERTY_MAP_HPP_
#define RUNIR_GRAPHS_PROPERTY_MAP_HPP_

#include "runir/graphs/declarations.hpp"

#include <tuple>
#include <tyr/common/declarations.hpp>
#include <tyr/common/index_mixins.hpp>
#include <tyr/common/indexed_hash_set.hpp>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <utility>

namespace tyr
{

template<runir::graphs::Property P>
struct Index<runir::graphs::VertexProperty<P>> : IndexMixin<Index<runir::graphs::VertexProperty<P>>>
{
    using Base = IndexMixin<Index<runir::graphs::VertexProperty<P>>>;
    using Base::Base;
};

template<runir::graphs::Property P>
struct Index<runir::graphs::EdgeProperty<P>> : IndexMixin<Index<runir::graphs::EdgeProperty<P>>>
{
    using Base = IndexMixin<Index<runir::graphs::EdgeProperty<P>>>;
    using Base::Base;
};

template<runir::graphs::Property P>
struct Data<runir::graphs::VertexProperty<P>>
{
    P value {};

    Data() = default;
    explicit Data(P value_) noexcept : value(std::move(value_)) {}

    void clear() noexcept { tyr::clear(value); }
    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<runir::graphs::Property P>
struct Data<runir::graphs::EdgeProperty<P>>
{
    P value {};

    Data() = default;
    explicit Data(P value_) noexcept : value(std::move(value_)) {}

    void clear() noexcept { tyr::clear(value); }
    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}  // namespace tyr

namespace runir::graphs
{

template<typename Tag>
class PropertyMap
{
private:
    tyr::IndexedHashSet<Tag> m_set;

public:
    using Index = tyr::Index<Tag>;
    using Data = tyr::Data<Tag>;

    PropertyMap() = default;

    void clear() noexcept { m_set.clear(); }

    auto get_or_create(const Data& data) { return m_set.insert(data); }

    auto get(Index index) const noexcept -> const Data& { return m_set[index]; }
    auto operator[](Index index) const noexcept -> const Data& { return get(index); }
    auto get_value(Index index) const noexcept -> const auto& { return get(index).value; }

    auto size() const noexcept -> std::size_t { return m_set.size(); }
};

template<Property P>
using VertexPropertyMap = PropertyMap<VertexProperty<P>>;

template<Property P>
using EdgePropertyMap = PropertyMap<EdgeProperty<P>>;

}  // namespace runir::graphs

#endif
