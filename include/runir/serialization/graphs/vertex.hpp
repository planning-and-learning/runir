#ifndef RUNIR_SERIALIZATION_GRAPHS_VERTEX_HPP_
#define RUNIR_SERIALIZATION_GRAPHS_VERTEX_HPP_

#include "runir/graphs/vertex.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<typename G, ::runir::graphs::Property P>
struct TypeName<::runir::graphs::Vertex<G, P>>
{
    static std::string get() { return "Vertex"; }
};

template<typename Archive, typename G, ::runir::graphs::Property P>
void describe_fields(Archive& ar, std::type_identity<::runir::graphs::Vertex<G, P>>)
{
    ar.field("index", [](const auto& value) -> decltype(auto) { return (value.get_index()); });
    ar.field("property", [](const auto& value) -> decltype(auto) { return (value.get_property()); });
}

}

#endif
