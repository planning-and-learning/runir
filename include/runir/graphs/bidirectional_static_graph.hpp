#ifndef RUNIR_GRAPHS_BIDIRECTIONAL_STATIC_GRAPH_HPP_
#define RUNIR_GRAPHS_BIDIRECTIONAL_STATIC_GRAPH_HPP_

#include "runir/graphs/backward_static_graph_view.hpp"

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

    explicit BidirectionalStaticGraph(const StaticGraphBuilder<VP, EP>& builder) : m_forward_graph(builder), m_backward_graph(m_forward_graph) {}

    explicit BidirectionalStaticGraph(StaticGraphBuilder<VP, EP>&& builder) : m_forward_graph(std::move(builder)), m_backward_graph(m_forward_graph) {}

    auto get_forward_graph() const noexcept -> const ForwardGraphType& { return m_forward_graph; }

    auto get_backward_graph() const noexcept -> const BackwardGraphType& { return m_backward_graph; }
};

}  // namespace runir::graphs

#endif
