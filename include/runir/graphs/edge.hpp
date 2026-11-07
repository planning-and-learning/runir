#ifndef RUNIR_GRAPHS_EDGE_HPP_
#define RUNIR_GRAPHS_EDGE_HPP_

#include "runir/graphs/declarations.hpp"

namespace runir::graphs
{

/// An edge: its index, endpoints, and its property's index in its graph's property pool (the graph's get_edge_property).
template<Property P>
class Edge : public urd::compared_by_identity<Edge<P>>
{
private:
    EdgeIndex m_index;
    VertexIndex m_source;
    VertexIndex m_target;
    EdgePropertyIndex<P> m_property;

public:
    using PropertyType = P;
    using PropertyIndexType = EdgePropertyIndex<P>;

    Edge(EdgeIndex index, VertexIndex source, VertexIndex target, PropertyIndexType property) noexcept :
        m_index(index),
        m_source(source),
        m_target(target),
        m_property(property)
    {
    }

    [[nodiscard]] auto get_index() const noexcept -> EdgeIndex { return m_index; }
    [[nodiscard]] auto get_source() const noexcept -> VertexIndex { return m_source; }
    [[nodiscard]] auto get_target() const noexcept -> VertexIndex { return m_target; }
    [[nodiscard]] auto get_property_index() const noexcept -> PropertyIndexType { return m_property; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&Edge::m_index, &Edge::m_source, &Edge::m_target, &Edge::m_property> {}; }
};

}  // namespace runir::graphs

#endif
