#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/serialization/kr/dl/semantics/boolean_view.hpp"
#include "runir/serialization/kr/dl/semantics/concept_view.hpp"
#include "runir/serialization/kr/dl/semantics/numerical_view.hpp"
#include "runir/serialization/kr/dl/semantics/role_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct TypeName<View<Index<runir::kr::dl::Constructor<Family, Category>>, C>>
{
    static std::string get() { return std::string(Family::name) + "." + Category::name + ".Constructor"; }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::dl::Constructor<Family, Category>>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar) { ar.variant(value.get_variant()); });
}

template<runir::kr::dl::FamilyTag Family, typename C>
struct TypeName<View<cista::offset::variant<Index<runir::kr::dl::Constructor<Family, runir::kr::dl::ConceptTag>>,
                                           Index<runir::kr::dl::Constructor<Family, runir::kr::dl::RoleTag>>>, C>>
{
    static std::string get() { return std::string(Family::name) + ".ConceptOrRole"; }
};

template<runir::kr::dl::FamilyTag Family, typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<cista::offset::variant<Index<runir::kr::dl::Constructor<Family, runir::kr::dl::ConceptTag>>,
                                                 Index<runir::kr::dl::Constructor<Family, runir::kr::dl::RoleTag>>>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar) { ar.variant(value); });
}

}

#endif
