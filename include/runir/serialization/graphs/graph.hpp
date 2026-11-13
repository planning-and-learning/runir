#ifndef RUNIR_SERIALIZATION_GRAPHS_GRAPH_HPP_
#define RUNIR_SERIALIZATION_GRAPHS_GRAPH_HPP_

#include "runir/datasets/graph_types.hpp"
#include "yggdrasil/serialization/conversion.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

#include <cstddef>
#include <ranges>
#include <utility>
#include <vector>

namespace ygg::serialization
{

/// A runir graph: its vertices' labels by vertex, its edges as (source, target) by edge number, and the edges' labels.
template<typename Archive, ::runir::labeled_graph G>
void describe_fields(Archive& ar, std::type_identity<G>)
{
    ar.field("vertices",
             [](const G& value) { return value.vertices() | std::views::transform([&value](auto v) -> decltype(auto) { return value.vertex_value(v); }); });
    ar.field("edges",
             [](const G& value)
             {
                 auto result = std::vector<std::pair<::runir::VertexIndex, ::runir::VertexIndex>>();
                 for (const auto e : value.edges())
                     result.emplace_back(value.source(e), value.target(e));
                 return result;
             });
    ar.field("edge_properties",
             [](const G& value) { return value.edges() | std::views::transform([&value](std::size_t e) -> decltype(auto) { return value.edge_value(e); }); });
}

}

#endif
