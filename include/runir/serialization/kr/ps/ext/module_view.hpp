#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_VIEW_HPP_

#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/serialization/kr/dl/argument_view.hpp"
#include "runir/serialization/kr/dl/register_view.hpp"
#include "runir/serialization/kr/ps/ext/rule_variant_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename C>
struct TypeName<View<Index<runir::kr::ps::ext::Module>, C>>
{
    static std::string get() { return "Ext.Module"; }
};

template<typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::ext::Module>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("symbol", value.get_symbol());
        ar.field("concept_arguments", value.template get_arguments<runir::kr::dl::ConceptTag>());
        ar.field("role_arguments", value.template get_arguments<runir::kr::dl::RoleTag>());
        ar.field("boolean_arguments", value.template get_arguments<runir::kr::dl::BooleanTag>());
        ar.field("numerical_arguments", value.template get_arguments<runir::kr::dl::NumericalTag>());
        ar.field("concept_registers", value.template get_registers<runir::kr::dl::ConceptTag>());
        ar.field("role_registers", value.template get_registers<runir::kr::dl::RoleTag>());
        ar.field("concept_features", value.template get_features<runir::kr::dl::ConceptTag>());
        ar.field("role_features", value.template get_features<runir::kr::dl::RoleTag>());
        ar.field("boolean_features", value.template get_features<runir::kr::ps::dl::BooleanFeature>());
        ar.field("numerical_features", value.template get_features<runir::kr::ps::dl::NumericalFeature>());
        ar.field("entry_memory_state", value.get_entry_memory_state());
        ar.field("memory_states", value.get_memory_states());
        ar.field("memory_transitions", value.get_memory_transitions());
    });
}

}

#endif
