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

template<::runir::graphs::Property VP, ::runir::graphs::Property EP>
void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::graphs::StaticGraph<VP, EP>& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("vertices", value.get_vertices());
        ar.field("edges", value.get_edges());
    });
}

}

#endif
