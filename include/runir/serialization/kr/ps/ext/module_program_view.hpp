#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_PROGRAM_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_PROGRAM_VIEW_HPP_

#include "runir/kr/ps/ext/module_program_view.hpp"
#include "runir/serialization/kr/ps/ext/module_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::ext::ModuleProgram>, C>>)
{
    ar.field("entry_module", [](const auto& value) -> decltype(auto) { return (value.get_entry_module()); });
    ar.field("modules", [](const auto& value) -> decltype(auto) { return (value.get_modules()); });
}

}

#endif
