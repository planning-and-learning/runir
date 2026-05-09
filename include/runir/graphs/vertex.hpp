#ifndef RUNIR_GRAPHS_VERTEX_HPP_
#define RUNIR_GRAPHS_VERTEX_HPP_

#include "runir/graphs/declarations.hpp"

#include <cassert>
#include <tuple>

namespace runir::graphs
{

template<typename G, Property P>
class Vertex
{
private:
    const G* m_graph;
    VertexIndex m_index;
    VertexPropertyIndex<P> m_property;

public:
    using GraphType = G;
    using PropertyType = P;
    using PropertyIndexType = VertexPropertyIndex<P>;

    Vertex(VertexIndex index, PropertyIndexType property, const G& graph) noexcept : m_graph(&graph), m_index(index), m_property(property) {}

    auto get_parent() const noexcept -> const G&
    {
        assert(m_graph);
        return *m_graph;
    }

    auto get_index() const noexcept -> VertexIndex { return m_index; }
    auto get_property_index() const noexcept -> PropertyIndexType { return m_property; }
    auto get_property() const noexcept -> const P& { return get_parent().get_vertex_property(m_property); }

    auto identifying_members() const noexcept { return std::tie(m_index, m_property); }
};

}  // namespace runir::graphs

#endif
