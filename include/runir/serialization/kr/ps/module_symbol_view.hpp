#ifndef RUNIR_SERIALIZATION_KR_PS_MODULE_SYMBOL_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_MODULE_SYMBOL_VIEW_HPP_

#include "runir/kr/ps/module_symbol_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, runir::kr::ps::ProgramFamilyTag Family, typename C>
void describe_fields(Archive& ar, std::type_identity<urd::index_view<runir::kr::ps::ModuleSymbol<Family>, C>>)
{
    ar.field("name", [](const auto& value) -> decltype(auto) { return (value.get_name()); });
}

}

#endif
