#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_DENOTATION_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_DENOTATION_VIEW_HPP_

#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <tyr/serialization/formalism/object_view.hpp>
#include <vector>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, runir::kr::dl::CategoryTag Category, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::semantics::Denotation<Category>>, C>>)
{
    if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag> || std::same_as<Category, runir::kr::dl::NumericalTag>)
    {
        ar.field("value", [](const auto& value) -> decltype(auto) { return (value.get()); });
    }
    else
    {
        ar.field("values", [](const auto& value)
        {
            auto elements = std::vector<runir::kr::dl::semantics::DenotationElement<Category, C>> {};
            for (auto element : value)
                elements.push_back(element);
            return elements;
        });
    }
}

}

#endif
