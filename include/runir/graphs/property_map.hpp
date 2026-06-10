#ifndef RUNIR_GRAPHS_PROPERTY_MAP_HPP_
#define RUNIR_GRAPHS_PROPERTY_MAP_HPP_

#include "runir/graphs/declarations.hpp"

#include <concepts>
#include <tuple>
#include <utility>
#include <yggdrasil/containers/indexed_hash_set.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/semantics/canonicalization.hpp>

namespace ygg
{

namespace detail
{
template<typename T>
concept HasIsCanonical = requires(const T& value) {
    { ygg::is_canonical(value) } -> std::same_as<bool>;
};

template<typename T>
concept HasCanonicalize = requires(T& value) { ygg::canonicalize(value); };
}

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

    void clear() noexcept { ygg::clear(value); }
    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<runir::graphs::Property P>
struct Data<runir::graphs::EdgeProperty<P>>
{
    P value {};

    Data() = default;
    explicit Data(P value_) noexcept : value(std::move(value_)) {}

    void clear() noexcept { ygg::clear(value); }
    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<runir::graphs::Property P>
bool is_canonical(const Data<runir::graphs::VertexProperty<P>>& data) noexcept
{
    if constexpr (detail::HasIsCanonical<P>)
        return ygg::is_canonical(data.value);
    else
        return true;
}

template<runir::graphs::Property P>
bool is_canonical(const Data<runir::graphs::EdgeProperty<P>>& data) noexcept
{
    if constexpr (detail::HasIsCanonical<P>)
        return ygg::is_canonical(data.value);
    else
        return true;
}

template<runir::graphs::Property P>
void canonicalize(Data<runir::graphs::VertexProperty<P>>& data) noexcept
{
    if constexpr (detail::HasCanonicalize<P>)
        ygg::canonicalize(data.value);
}

template<runir::graphs::Property P>
void canonicalize(Data<runir::graphs::EdgeProperty<P>>& data) noexcept
{
    if constexpr (detail::HasCanonicalize<P>)
        ygg::canonicalize(data.value);
}

}  // namespace ygg

namespace runir::graphs
{

template<typename Tag>
class PropertyMap
{
private:
    ygg::IndexedHashSet<Tag> m_set;

public:
    using Index = ygg::Index<Tag>;
    using Data = ygg::Data<Tag>;

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
