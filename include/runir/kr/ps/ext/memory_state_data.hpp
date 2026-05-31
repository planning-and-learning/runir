#ifndef RUNIR_KR_PS_EXT_MEMORY_STATE_DATA_HPP_
#define RUNIR_KR_PS_EXT_MEMORY_STATE_DATA_HPP_

#include "runir/kr/ps/ext/memory_state_index.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ext::MemoryState>
{
    Index<runir::kr::ps::ext::MemoryState> index;
    ::cista::offset::string name;

    Data() = default;
    Data(::cista::offset::string name_) : index(), name(std::move(name_)) {}
    Data(const std::string& name_) : index(), name(name_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(name);
    }

    auto cista_members() const noexcept { return std::tie(index, name); }
    auto identifying_members() const noexcept { return std::tie(name); }
};

}  // namespace ygg

#endif
