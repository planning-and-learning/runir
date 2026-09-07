#ifndef RUNIR_SERIALIZATION_GRAPHS_EDGE_HPP_
#define RUNIR_SERIALIZATION_GRAPHS_EDGE_HPP_

#include "runir/graphs/edge.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<typename Archive, typename G, ::runir::graphs::Property P>
void describe_fields(Archive& ar, std::type_identity<::runir::graphs::Edge<G, P>>)
{
    ar.field("index", [](const auto& value) -> decltype(auto) { return (value.get_index()); });
    ar.field("source", [](const auto& value) -> decltype(auto) { return (value.get_source()); });
    ar.field("target", [](const auto& value) -> decltype(auto) { return (value.get_target()); });
    ar.field("property", [](const auto& value) -> decltype(auto) { return (value.get_property()); });
}

}

#endif
