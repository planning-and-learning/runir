#include <fmt/format.h>
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <runir/kr/uns.hpp>
#include <runir/runir.hpp>
#include <string>
#include <tuple>
#include <urd/repr/graphs/csr_graph.hpp>
#include <vector>
#include <vefr/dot.hpp>

namespace nb = nanobind;

namespace
{

std::string make_graph_dot()
{
    // runir's graphs are urd graphs with their labels inside; vefr writes one as DOT.
    using Edge = std::tuple<runir::VertexIndex, runir::VertexIndex, std::string>;
    const auto edges = std::vector<Edge> { { 0, 1, "edge" } };
    const auto names = std::vector<std::string> { "source", "target" };
    const auto graph = urd::Store<urd::graph<runir::VertexIndex, std::string, std::string>, urd::frozen>(urd::graph_data<Edge, std::string> { 2, edges, {}, names });
    return vefr::to_dot(graph, vefr::vertex_data, vefr::edge_data);
}

}  // namespace

NB_MODULE(downstream_runir_user, m) { m.def("make_graph_dot", &make_graph_dot); }
