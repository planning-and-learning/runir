#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_DENOTATION_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_DENOTATION_VIEW_HPP_

#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <tyr/serialization/formalism/object_view.hpp>
#include <vector>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<runir::kr::dl::CategoryTag Category, typename C>
struct TypeName<View<Index<runir::kr::dl::semantics::Denotation<Category>>, C>>
{
    static std::string get() { return std::string(Category::name) + "Denotation"; }
};

template<runir::kr::dl::CategoryTag Category, typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::dl::semantics::Denotation<Category>>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag> || std::same_as<Category, runir::kr::dl::NumericalTag>)
        {
            ar.field("value", value.get());
        }
        else
        {
            auto elements = std::vector<runir::kr::dl::semantics::DenotationElement<Category, C>> {};
            for (auto element : value)
                elements.push_back(element);
            ar.field("values", elements);
        }
    });
}

}

#endif
