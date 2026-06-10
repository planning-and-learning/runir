#ifndef RUNIR_GRAPHS_BIDIRECTIONAL_STATIC_GRAPH_HPP_
#define RUNIR_GRAPHS_BIDIRECTIONAL_STATIC_GRAPH_HPP_

#include "runir/graphs/backward_static_graph_view.hpp"

#include <utility>

namespace runir::graphs
{

template<Property VP = std::tuple<>, Property EP = std::tuple<>>
class BidirectionalStaticGraph
{
public:
    using VertexPropertyType = VP;
    using EdgePropertyType = EP;
    using ForwardGraphType = StaticGraph<VP, EP>;
    using BackwardGraphType = BackwardStaticGraphView<ForwardGraphType>;

private:
    ForwardGraphType m_forward_graph;
    BackwardGraphType m_backward_graph;

public:
    BidirectionalStaticGraph() : m_forward_graph(), m_backward_graph(m_forward_graph) {}

    BidirectionalStaticGraph(const BidirectionalStaticGraph&) = delete;

    BidirectionalStaticGraph(BidirectionalStaticGraph&& other) : m_forward_graph(std::move(other.m_forward_graph)), m_backward_graph(m_forward_graph) {}

    auto operator=(const BidirectionalStaticGraph&) -> BidirectionalStaticGraph& = delete;

    auto operator=(BidirectionalStaticGraph&& other) -> BidirectionalStaticGraph&
    {
        if (this == &other)
            return *this;

        m_forward_graph = std::move(other.m_forward_graph);
        m_backward_graph = BackwardGraphType(m_forward_graph);
        return *this;
    }

    explicit BidirectionalStaticGraph(const StaticGraphBuilder<VP, EP>& builder) : m_forward_graph(builder), m_backward_graph(m_forward_graph) {}

    explicit BidirectionalStaticGraph(StaticGraphBuilder<VP, EP>&& builder) : m_forward_graph(std::move(builder)), m_backward_graph(m_forward_graph) {}

    auto get_forward_graph() const noexcept -> const ForwardGraphType& { return m_forward_graph; }

    auto get_backward_graph() const noexcept -> const BackwardGraphType& { return m_backward_graph; }
};

}  // namespace runir::graphs

#endif
