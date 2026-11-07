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

/// The distinct properties of a graph's vertices and edges, interned in a urd pool: a handle is a property's index.
template<Property P>
using VertexPropertyMap = urd::Store<urd::pool<urd::data<VertexProperty<P>>>>;

template<Property P>
using EdgePropertyMap = urd::Store<urd::pool<urd::data<EdgeProperty<P>>>>;

}  // namespace runir::graphs

#endif
