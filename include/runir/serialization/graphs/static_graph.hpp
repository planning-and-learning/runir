#ifndef RUNIR_SERIALIZATION_GRAPHS_STATIC_GRAPH_HPP_
#define RUNIR_SERIALIZATION_GRAPHS_STATIC_GRAPH_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/serialization/graphs/vertex.hpp"
#include "runir/serialization/graphs/edge.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<::runir::graphs::Property VP, ::runir::graphs::Property EP>
struct TypeName<::runir::graphs::StaticGraph<VP, EP>>
{
    static std::string get() { return "StaticGraph"; }
};

template<typename Archive, ::runir::graphs::Property VP, ::runir::graphs::Property EP>
void describe_fields(Archive& ar, std::type_identity<::runir::graphs::StaticGraph<VP, EP>>)
{
    ar.field("vertices", [](const auto& value) -> decltype(auto) { return (value.get_vertices()); });
    ar.field("edges", [](const auto& value) -> decltype(auto) { return (value.get_edges()); });
}

}

#endif
