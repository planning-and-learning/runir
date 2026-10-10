#ifndef RUNIR_KR_PS_EXT_PROGRAM_DATA_HPP_
#define RUNIR_KR_PS_EXT_PROGRAM_DATA_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ext::Program>
{
    Index<runir::kr::ps::ext::Program> index;
    Index<runir::kr::ps::ext::Module> entry_module;
    IndexList<runir::kr::ps::ext::Module> modules;

    Data() = default;
    Data(Index<runir::kr::ps::ext::Module> entry_module_, IndexList<runir::kr::ps::ext::Module> modules_) :
        index(),
        entry_module(std::move(entry_module_)),
        modules(std::move(modules_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::ps::ext::Module>, C> entry_module_, const std::vector<::ygg::View<Index<runir::kr::ps::ext::Module>, C>>& modules_) :
        index(),
        entry_module(),
        modules()
    {
        set(entry_module_, entry_module);
        set(modules_, modules);
    }

    auto cista_members() noexcept { return std::tie(index, entry_module, modules); }
    auto cista_members() const noexcept { return std::tie(index, entry_module, modules); }
    auto identifying_members() const noexcept { return std::tie(entry_module, modules); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
