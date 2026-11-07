#ifndef RUNIR_GRAPHS_VERTEX_HPP_
#define RUNIR_GRAPHS_VERTEX_HPP_

#include "runir/graphs/declarations.hpp"

namespace runir::graphs
{

/// A vertex: its index and its property's index in its graph's property pool (the graph's get_vertex_property).
template<Property P>
class Vertex : public urd::compared_by_identity<Vertex<P>>
{
private:
    VertexIndex m_index;
    VertexPropertyIndex<P> m_property;

public:
    using PropertyType = P;
    using PropertyIndexType = VertexPropertyIndex<P>;

    Vertex(VertexIndex index, PropertyIndexType property) noexcept : m_index(index), m_property(property) {}

    [[nodiscard]] auto get_index() const noexcept -> VertexIndex { return m_index; }
    [[nodiscard]] auto get_property_index() const noexcept -> PropertyIndexType { return m_property; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&Vertex::m_index, &Vertex::m_property> {}; }
};

}  // namespace runir::graphs

#endif
