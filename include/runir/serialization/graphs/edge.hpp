#ifndef RUNIR_SERIALIZATION_GRAPHS_EDGE_HPP_
#define RUNIR_SERIALIZATION_GRAPHS_EDGE_HPP_

#include "runir/graphs/edge.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<typename G, ::runir::graphs::Property P>
struct TypeName<::runir::graphs::Edge<G, P>>
{
    static std::string get() { return "Edge"; }
};

template<typename G, ::runir::graphs::Property P>
void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::graphs::Edge<G, P>& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("index", value.get_index());
        ar.field("source", value.get_source());
        ar.field("target", value.get_target());
        ar.field("property", value.get_property());
    });
}

}

#endif
