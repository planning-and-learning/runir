#ifndef RUNIR_KR_PS_ICP_PROGRAM_DATA_HPP_
#define RUNIR_KR_PS_ICP_PROGRAM_DATA_HPP_

#include "runir/kr/ps/icp/module_index.hpp"
#include "runir/kr/ps/icp/program_index.hpp"

#include <tuple>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::icp::Program>
{
    Index<runir::kr::ps::icp::Program> index;
    Index<runir::kr::ps::icp::Module> module;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(module);
    }

    auto cista_members() const noexcept { return std::tie(index, module); }
    auto identifying_members() const noexcept { return std::tie(module); }
};

}  // namespace ygg

#endif
