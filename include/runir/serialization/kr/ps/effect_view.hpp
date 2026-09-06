#ifndef RUNIR_SERIALIZATION_KR_PS_EFFECT_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EFFECT_VIEW_HPP_

#include "runir/kr/ps/effect_view.hpp"
#include "runir/serialization/kr/ps/base/dl/effect_view.hpp"
#include "runir/serialization/kr/ps/ext/dl/effect_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<runir::kr::FamilyTag Family, typename C>
struct TypeName<View<Index<runir::kr::ps::EffectVariant<Family>>, C>>
{
    static std::string get() { return std::string(Family::name) + ".Effect"; }
};

template<runir::kr::FamilyTag Family, typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::EffectVariant<Family>>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.variant(value.get_variant());
    });
}

template<runir::kr::FamilyTag Family, typename C>
struct TypeName<View<Index<runir::kr::ps::ConcreteEffectVariant<Family, runir::kr::DlTag>>, C>>
{
    static std::string get() { return std::string(Family::name) + ".DL.Effect"; }
};

template<runir::kr::FamilyTag Family, typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::ConcreteEffectVariant<Family, runir::kr::DlTag>>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.variant(value.get_variant());
    });
}

}

#endif
