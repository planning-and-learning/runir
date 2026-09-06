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

template<typename G, ::runir::graphs::Property P>
void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::graphs::Vertex<G, P>& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("index", value.get_index());
        ar.field("property", value.get_property());
    });
}

}

#endif
