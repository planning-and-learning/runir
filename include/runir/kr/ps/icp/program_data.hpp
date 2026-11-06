#ifndef RUNIR_KR_PS_ICP_PROGRAM_DATA_HPP_
#define RUNIR_KR_PS_ICP_PROGRAM_DATA_HPP_

#include "runir/kr/ps/icp/declarations.hpp"

#include <tuple>
#include <utility>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<runir::kr::ps::icp::Program>
{
    urd::index<runir::kr::ps::icp::Program> index;
    urd::index<runir::kr::ps::icp::Module> module;

    data() = default;
    data(urd::index<runir::kr::ps::icp::Module> module_) : index(), module(module_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::module> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::module> {}; }
};

}  // namespace urd

#endif
