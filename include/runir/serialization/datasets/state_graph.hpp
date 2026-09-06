#ifndef RUNIR_SERIALIZATION_DATASETS_STATE_GRAPH_HPP_
#define RUNIR_SERIALIZATION_DATASETS_STATE_GRAPH_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/serialization/graphs/bidirectional_static_graph.hpp"
#include "tyr/serialization/planning/state_view.hpp"
#include "tyr/serialization/formalism/binding_view.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<::tyr::TaskKind Kind>
struct TypeName<::runir::datasets::StateGraphVertexLabel<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "StateGraphVertexLabel"; }
};

template<::tyr::TaskKind Kind>
void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::datasets::StateGraphVertexLabel<Kind>& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("state", value.state);
    });
}

template<::tyr::TaskKind Kind>
struct TypeName<::runir::datasets::AnnotatedStateGraphVertexLabel<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "AnnotatedStateGraphVertexLabel"; }
};

template<::tyr::TaskKind Kind>
void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::datasets::AnnotatedStateGraphVertexLabel<Kind>& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("state", value.state);
        ar.field("goal_distance", value.goal_distance);
        ar.field("is_initial", value.is_initial);
        ar.field("is_goal", value.is_goal);
        ar.field("is_alive", value.is_alive);
        ar.field("is_unsolvable", value.is_unsolvable);
    });
}

template<>
struct TypeName<::runir::datasets::StateGraphEdgeLabel>
{
    static std::string get() { return "StateGraphEdgeLabel"; }
};

inline void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::datasets::StateGraphEdgeLabel& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("action", value.action);
        ar.field("cost", value.cost);
    });
}

}

#endif
