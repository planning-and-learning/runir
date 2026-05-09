#ifndef RUNIR_GRAPHS_EDGE_HPP_
#define RUNIR_GRAPHS_EDGE_HPP_

#include "runir/graphs/declarations.hpp"

#include <cassert>
#include <tuple>

namespace runir::graphs
{

template<typename G, Property P>
class Edge
{
private:
    const G* m_graph;
    EdgeIndex m_index;
    VertexIndex m_source;
    VertexIndex m_target;
    EdgePropertyIndex<P> m_property;

public:
    using GraphType = G;
    using PropertyType = P;
    using PropertyIndexType = EdgePropertyIndex<P>;

    Edge(EdgeIndex index, VertexIndex source, VertexIndex target, PropertyIndexType property, const G& graph) noexcept :
        m_graph(&graph),
        m_index(index),
        m_source(source),
        m_target(target),
        m_property(property)
    {
    }

    auto get_parent() const noexcept -> const G&
    {
        assert(m_graph);
        return *m_graph;
    }

    auto get_index() const noexcept -> EdgeIndex { return m_index; }
    auto get_source() const noexcept -> VertexIndex { return m_source; }
    auto get_target() const noexcept -> VertexIndex { return m_target; }
    auto get_property_index() const noexcept -> PropertyIndexType { return m_property; }
    auto get_property() const noexcept -> const P& { return get_parent().get_edge_property(m_property); }

    auto identifying_members() const noexcept { return std::tie(m_index, m_source, m_target, m_property); }
};

}  // namespace runir::graphs

#endif
