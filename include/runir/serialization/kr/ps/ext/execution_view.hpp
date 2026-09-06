#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_EXECUTION_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_EXECUTION_VIEW_HPP_

#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/serialization/kr/dl/semantics/denotation_view.hpp"
#include "runir/serialization/kr/ps/ext/execution_declarations.hpp"
#include "runir/serialization/kr/ps/ext/module_program_view.hpp"
#include "tyr/planning/state_repository.hpp"
#include "tyr/serialization/formalism/object_view.hpp"
#include "tyr/serialization/planning/state_view.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<::tyr::TaskKind Kind>
struct TypeName<::runir::kr::ps::ext::RegisterValuesView<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "RegisterValues"; }
};

template<::tyr::TaskKind Kind>
void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::kr::ps::ext::RegisterValuesView<Kind>& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("concept_values", value.get_concept_values());
        ar.field("role_values", value.get_role_values());
    });
}

template<::tyr::TaskKind Kind>
struct TypeName<::runir::kr::ps::ext::CallArgumentsView<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "CallArguments"; }
};

template<::tyr::TaskKind Kind>
void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::kr::ps::ext::CallArgumentsView<Kind>& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("concept_arguments", value.template get<::runir::kr::dl::ConceptTag>());
        ar.field("role_arguments", value.template get<::runir::kr::dl::RoleTag>());
        ar.field("boolean_arguments", value.template get<::runir::kr::dl::BooleanTag>());
        ar.field("numerical_arguments", value.template get<::runir::kr::dl::NumericalTag>());
    });
}

template<::tyr::TaskKind Kind>
struct TypeName<::runir::kr::ps::ext::CallStackView<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "CallStack"; }
};

template<::tyr::TaskKind Kind>
void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::kr::ps::ext::CallStackView<Kind>& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("module", value.get_module());
        ar.field("memory_state", value.get_memory_state());
        ar.field("registers", value.get_registers());
        ar.field("arguments", value.get_arguments());
        ar.field("caller", value.get_caller());
    });
}

template<::tyr::TaskKind Kind>
struct TypeName<::runir::kr::ps::ext::ExecutionStateView<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "ExecutionState"; }
};

template<::tyr::TaskKind Kind>
void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                const ::runir::kr::ps::ext::ExecutionStateView<Kind>& value, Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("state", value.get_state());
        ar.field("program", value.get_program());
        ar.field("phase", value.get_phase());
        ar.field("call_stack", value.get_call_stack());
    });
}


}

#endif
