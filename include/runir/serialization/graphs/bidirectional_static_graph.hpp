#ifndef RUNIR_SERIALIZATION_GRAPHS_BIDIRECTIONAL_STATIC_GRAPH_HPP_
#define RUNIR_SERIALIZATION_GRAPHS_BIDIRECTIONAL_STATIC_GRAPH_HPP_

#include "runir/graphs/bidirectional_static_graph.hpp"
#include "runir/serialization/graphs/static_graph.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<typename Archive, ::runir::graphs::Property VP, ::runir::graphs::Property EP>
void describe_fields(Archive& ar, std::type_identity<::runir::graphs::BidirectionalStaticGraph<VP, EP>>)
{
    using ForwardGraph = typename ::runir::graphs::BidirectionalStaticGraph<VP, EP>::ForwardGraphType;
    describe_fields(ar, std::type_identity<ForwardGraph> {});
}

template<::runir::graphs::Property VP, ::runir::graphs::Property EP>
void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::graphs::BidirectionalStaticGraph<VP, EP>& value, Dictionaries* dictionaries)
{
    result = boost::json::value_from(value.get_forward_graph(), dictionaries);
}

}

#endif
