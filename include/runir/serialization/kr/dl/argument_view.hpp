#ifndef RUNIR_SERIALIZATION_KR_DL_ARGUMENT_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_ARGUMENT_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/repository.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, runir::kr::dl::CategoryTag Category, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::Argument<Category>>, C>>)
{
    ar.field("name", [](const auto& value) -> decltype(auto) { return (value.get_name()); });
    ar.field("identifier", [](const auto& value) -> decltype(auto) { return (uint_t(value.get_identifier())); });
}

}

#endif
