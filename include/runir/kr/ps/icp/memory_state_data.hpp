#ifndef RUNIR_KR_PS_ICP_MEMORY_STATE_DATA_HPP_
#define RUNIR_KR_PS_ICP_MEMORY_STATE_DATA_HPP_

#include "runir/kr/ps/icp/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::icp::MemoryState>
{
    Index<runir::kr::ps::icp::MemoryState> index;
    ::cista::offset::string name;

    Data() = default;
    Data(::cista::offset::string name_) : index(), name(std::move(name_)) {}

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::name> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::name> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

}  // namespace ygg

#endif
