#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <runir/runir.hpp>
#include <string>

namespace nb = nanobind;

namespace
{

std::string make_graph_dot()
{
    auto builder = runir::graphs::StaticGraphBuilder<std::string, std::string>();
    const auto source = builder.add_vertex("source");
    const auto target = builder.add_vertex("target");
    builder.add_directed_edge(source, target, "edge");
    return runir::to_string(builder);
}

}  // namespace

NB_MODULE(downstream_runir_user, m)
{
    m.def("make_graph_dot", &make_graph_dot);
}
