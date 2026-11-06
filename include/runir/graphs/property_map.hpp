#ifndef RUNIR_GRAPHS_PROPERTY_MAP_HPP_
#define RUNIR_GRAPHS_PROPERTY_MAP_HPP_

#include "runir/graphs/declarations.hpp"

#include <concepts>
#include <tuple>
#include <utility>
#include <urd/repr/records/records.hpp>
#include <urd/store.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/semantics/canonicalization.hpp>

namespace urd
{

template<runir::graphs::Property P>
struct data<runir::graphs::VertexProperty<P>>
{
    P value {};

    data() = default;
    explicit data(P value_) noexcept : value(std::move(value_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::value> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::value> {}; }
};

template<runir::graphs::Property P>
struct data<runir::graphs::EdgeProperty<P>>
{
    P value {};

    data() = default;
    explicit data(P value_) noexcept : value(std::move(value_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::value> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::value> {}; }
};

}  // namespace urd

namespace runir::graphs
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
bool is_canonical(const urd::data<runir::graphs::VertexProperty<P>>& data) noexcept
{
    if constexpr (detail::HasIsCanonical<P>)
        return ygg::is_canonical(data.value);
    else
        return true;
}

template<runir::graphs::Property P>
bool is_canonical(const urd::data<runir::graphs::EdgeProperty<P>>& data) noexcept
{
    if constexpr (detail::HasIsCanonical<P>)
        return ygg::is_canonical(data.value);
    else
        return true;
}

template<runir::graphs::Property P>
void canonicalize(urd::data<runir::graphs::VertexProperty<P>>& data) noexcept
{
    if constexpr (detail::HasCanonicalize<P>)
        ygg::canonicalize(data.value);
}

template<runir::graphs::Property P>
void canonicalize(urd::data<runir::graphs::EdgeProperty<P>>& data) noexcept
{
    if constexpr (detail::HasCanonicalize<P>)
        ygg::canonicalize(data.value);
}

template<typename Tag>
class PropertyMap
{
private:
public:
    using Index = urd::index<Tag>;
    using Data = urd::data<Tag>;

private:
    /// The distinct properties, interned in a urd pool: a handle's value is the property's index.
    using Pool = urd::Store<urd::pool<Data>>;
    Pool m_set { urd::layer { 0 } };

public:

    PropertyMap() = default;

    void clear() noexcept { m_set.clear(); }

    std::pair<Index, bool> insert(const Data& data)
    {
        const auto [handle, inserted] = m_set.insert(data);
        return { Index(handle.value), inserted };
    }

    [[nodiscard]] auto get(Index index) const noexcept -> const Data& { return m_set[typename Pool::handle_type(index.value)]; }
    auto operator[](Index index) const noexcept -> const Data& { return get(index); }
    [[nodiscard]] auto get_value(Index index) const noexcept -> const auto& { return get(index).value; }

    [[nodiscard]] auto size() const noexcept -> std::size_t { return m_set.size(); }
};

template<Property P>
using VertexPropertyMap = PropertyMap<VertexProperty<P>>;

template<Property P>
using EdgePropertyMap = PropertyMap<EdgeProperty<P>>;

}  // namespace runir::graphs

#endif
