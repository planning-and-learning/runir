#ifndef RUNIR_SERIALIZATION_GRAPHS_STATIC_GRAPH_HPP_
#define RUNIR_SERIALIZATION_GRAPHS_STATIC_GRAPH_HPP_

#include "runir/graphs/static_graph.hpp"
#include "yggdrasil/serialization/conversion.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

#include <ranges>
#include <utility>

namespace ygg::serialization
{

/// The vertices' properties by vertex index, the edges as (source, target) by edge index, and the edges' properties.
template<typename Archive, ::runir::graphs::Property VP, ::runir::graphs::Property EP>
void describe_fields(Archive& ar, std::type_identity<::runir::graphs::StaticGraph<VP, EP>>)
{
    ar.field(
        "vertices",
        [](const auto& value)
        { return value.get_vertex_indices() | std::views::transform([&value](auto vertex) -> decltype(auto) { return value.get_vertex_property(vertex); }); });
    ar.field("edges",
             [](const auto& value)
             {
                 return value.get_edge_indices()
                        | std::views::transform([&value](auto edge) { return std::pair(value.get_source(edge), value.get_target(edge)); });
             });
    ar.field("edge_properties",
             [](const auto& value)
             { return value.get_edge_indices() | std::views::transform([&value](auto edge) -> decltype(auto) { return value.get_edge_property(edge); }); });
}

}

#endif
