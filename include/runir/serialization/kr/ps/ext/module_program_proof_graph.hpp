#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_

#include "runir/kr/ps/ext/module_program_proof_graph.hpp"
#include "runir/serialization/graphs/static_graph.hpp"
#include "runir/serialization/kr/ps/ext/execution_view.hpp"
#include "runir/serialization/kr/ps/ext/rule_variant_view.hpp"
#include "tyr/serialization/formalism/binding_view.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<::tyr::TaskKind Kind>
struct TypeName<::runir::kr::ps::ext::ModuleProgramProofVertexLabel<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "ModuleProgramProofVertexLabel"; }
};

template<::tyr::TaskKind Kind>
void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::kr::ps::ext::ModuleProgramProofVertexLabel<Kind>& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("execution_state", value.execution_state);
        ar.field("is_initial", value.is_initial);
        ar.field("is_goal", value.is_goal);
        ar.field("is_alive", value.is_alive);
        ar.field("is_unsolvable", value.is_unsolvable);
    });
}

template<>
struct TypeName<::runir::kr::ps::ext::ModuleProgramProofStateTransition>
{
    static std::string get() { return "ModuleProgramProofStateTransition"; }
};

inline void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::kr::ps::ext::ModuleProgramProofStateTransition& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("action", value.action);
        ar.field("cost", value.cost);
    });
}

template<>
struct TypeName<::runir::kr::ps::ext::ModuleProgramProofEdgeLabel>
{
    static std::string get() { return "ModuleProgramProofEdgeLabel"; }
};

inline void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::kr::ps::ext::ModuleProgramProofEdgeLabel& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("state_transition", value.state_transition);
        ar.field("rule", value.rule);
    });
}

}

#endif
