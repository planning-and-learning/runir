#ifndef RUNIR_SERIALIZATION_KR_PS_BASE_SKETCH_PROOF_GRAPH_HPP_
#define RUNIR_SERIALIZATION_KR_PS_BASE_SKETCH_PROOF_GRAPH_HPP_

#include "runir/kr/ps/base/sketch_proof_graph.hpp"
#include "runir/serialization/datasets/state_graph.hpp"
#include "runir/serialization/kr/ps/base/rule_view.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<::tyr::TaskKind Kind>
struct TypeName<::runir::kr::ps::base::SketchProofVertexLabel<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "SketchProofVertexLabel"; }
};

template<::tyr::TaskKind Kind>
void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::kr::ps::base::SketchProofVertexLabel<Kind>& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("state", value.state);
        ar.field("is_initial", value.is_initial);
        ar.field("is_goal", value.is_goal);
        ar.field("is_alive", value.is_alive);
        ar.field("is_unsolvable", value.is_unsolvable);
    });
}

template<>
struct TypeName<::runir::kr::ps::base::SketchProofEdgeLabel>
{
    static std::string get() { return "SketchProofEdgeLabel"; }
};

inline void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::kr::ps::base::SketchProofEdgeLabel& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("transition", value.transition);
        ar.field("rule", value.rule);
    });
}

}

#endif
