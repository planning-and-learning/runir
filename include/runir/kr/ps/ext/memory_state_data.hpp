#ifndef RUNIR_KR_PS_EXT_MEMORY_STATE_DATA_HPP_
#define RUNIR_KR_PS_EXT_MEMORY_STATE_DATA_HPP_

#include "runir/kr/ps/ext/memory_state_index.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
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
        tyr::clear(index);
        tyr::clear(name);
    }

    auto cista_members() const noexcept { return std::tie(index, name); }
    auto identifying_members() const noexcept { return std::tie(name); }
};

}  // namespace tyr

#endif
