#ifndef RUNIR_KR_PS_ICP_PROGRAM_DATA_HPP_
#define RUNIR_KR_PS_ICP_PROGRAM_DATA_HPP_

#include "runir/kr/ps/icp/declarations.hpp"

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

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::module> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::module> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
    }
};

}  // namespace ygg

#endif
