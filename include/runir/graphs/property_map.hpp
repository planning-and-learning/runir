#ifndef RUNIR_GRAPHS_PROPERTY_MAP_HPP_
#define RUNIR_GRAPHS_PROPERTY_MAP_HPP_

#include "runir/graphs/declarations.hpp"

#include <concepts>
#include <tuple>
#include <type_traits>
#include <urd/elements/flat.hpp>
#include <urd/repr/records/records.hpp>
#include <urd/store.hpp>
#include <utility>
#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::graphs::detail
{

/// The property type P of VertexProperty<P> and EdgeProperty<P>.
template<typename T>
struct PropertyOf;
template<Property P>
struct PropertyOf<VertexProperty<P>> : std::type_identity<P>
{
};
template<Property P>
struct PropertyOf<EdgeProperty<P>> : std::type_identity<P>
{
};

/// What names a graph's properties: VertexProperty<P> or EdgeProperty<P>.
template<typename T>
concept PropertyTag = requires { typename PropertyOf<T>::type; };

template<typename T>
concept HasIsCanonical = requires(const T& value) {
    { ygg::is_canonical(value) } -> std::same_as<bool>;
};

template<typename T>
concept HasCanonicalize = requires(T& value) { ygg::canonicalize(value); };

}  // namespace runir::graphs::detail

namespace urd
{

/// A vertex's or an edge's property, as its graph's property pool holds it.
template<runir::graphs::detail::PropertyTag T>
struct data<T>
{
    typename runir::graphs::detail::PropertyOf<T>::type value {};

    data() = default;
    explicit data(decltype(value) value_) noexcept : value(std::move(value_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::value> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::value> {}; }
};

}  // namespace urd

namespace runir::graphs
{

template<detail::PropertyTag T>
bool is_canonical(const urd::data<T>& data) noexcept
{
    if constexpr (detail::HasIsCanonical<decltype(data.value)>)
        return ygg::is_canonical(data.value);
    else
        return true;
}

template<detail::PropertyTag T>
void canonicalize(urd::data<T>& data) noexcept
{
    if constexpr (detail::HasCanonicalize<decltype(data.value)>)
        ygg::canonicalize(data.value);
}

/// The distinct properties of a graph's vertices and edges, interned in a urd pool: a handle is a property's index.
template<Property P>
using VertexPropertyMap = urd::Store<urd::pool<urd::data<VertexProperty<P>>>>;

template<Property P>
using EdgePropertyMap = urd::Store<urd::pool<urd::data<EdgeProperty<P>>>>;

}  // namespace runir::graphs

#endif
