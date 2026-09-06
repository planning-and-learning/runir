#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_RULE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_RULE_VIEW_HPP_

#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/serialization/kr/dl/register_view.hpp"
#include "runir/serialization/kr/ps/condition_view.hpp"
#include "runir/serialization/kr/ps/effect_view.hpp"
#include "runir/serialization/kr/ps/ext/memory_state_view.hpp"
#include "runir/serialization/kr/ps/ext/module_symbol_view.hpp"
#include "runir/serialization/kr/ps/ext/rule_data.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<runir::kr::ps::ext::RuleKind Kind, typename C>
struct TypeName<View<Index<runir::kr::ps::ext::Rule<Kind>>, C>>
{
    static std::string get()
    {
        auto prefix = std::string(runir::kr::ExtFamilyTag::name);
        if constexpr (std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::ConceptTag>>)
            prefix += std::string(".") + runir::kr::dl::ConceptTag::name;
        else if constexpr (std::same_as<Kind, runir::kr::ps::ext::LoadTag<runir::kr::dl::RoleTag>>)
            prefix += std::string(".") + runir::kr::dl::RoleTag::name;
        return prefix + "." + Kind::keyword + ".Rule";
    }
};

template<runir::kr::ps::ext::RuleKind Kind, typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::ext::Rule<Kind>>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("source", value.get_source());
        ar.field("target", value.get_target());
        ar.field("conditions", value.get_conditions());
        if constexpr (requires { value.get_effects(); })
            ar.field("effects", value.get_effects());
        if constexpr (requires { value.get_feature(); })
        {
            ar.field("feature", value.get_feature());
            ar.field("register", value.get_register());
        }
        if constexpr (requires { value.get_action_name(); })
        {
            ar.field("action_name", value.get_action_name());
            ar.field("arguments", value.get_action_arguments());
        }
        if constexpr (requires { value.get_callee(); })
        {
            ar.field("callee", value.get_callee());
            ar.field("arguments", make_view(value.get_data().arguments, value.get_context()));
        }
    });
}

}

#endif
