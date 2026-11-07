#ifndef RUNIR_KR_PS_MEMORY_STATE_DATA_HPP_
#define RUNIR_KR_PS_MEMORY_STATE_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <urd/elements/flat.hpp>

namespace urd
{

template<runir::kr::ps::ProgramFamilyTag Family>
struct data<runir::kr::ps::MemoryState<Family>>
{
    urd::index<runir::kr::ps::MemoryState<Family>> index;
    urd::flat_string name;

    data() = default;
    data(urd::flat_string name_) : index(), name(std::move(name_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::name> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::name> {}; }
};

}  // namespace urd

#endif
