#ifndef RUNIR_SERIALIZATION_KR_PS_ICP_PROGRAM_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_ICP_PROGRAM_VIEW_HPP_

#include "runir/kr/ps/icp/program_view.hpp"
#include "runir/serialization/kr/ps/icp/module_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::icp::Program>, C>>)
{
    ar.field("module", [](const auto& value) { return value.get_module(); });
}

}

#endif
