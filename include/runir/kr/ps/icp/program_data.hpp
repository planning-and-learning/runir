#ifndef RUNIR_KR_PS_ICP_PROGRAM_DATA_HPP_
#define RUNIR_KR_PS_ICP_PROGRAM_DATA_HPP_

#include "runir/kr/ps/icp/module_index.hpp"
#include "runir/kr/ps/icp/program_index.hpp"

#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::icp::Program>
{
    Index<runir::kr::ps::icp::Program> index;
    Index<runir::kr::ps::icp::Module> module;

    Data() = default;
    Data(Index<runir::kr::ps::icp::Module> module_) : index(), module(std::move(module_)) {}
    template<typename C>
    Data(::ygg::View<Index<runir::kr::ps::icp::Module>, C> module_) : index(), module()
    {
        set(module_, module);
    }

    auto cista_members() noexcept { return std::tie(index, module); }
    auto cista_members() const noexcept { return std::tie(index, module); }
    auto identifying_members() const noexcept { return std::tie(module); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
