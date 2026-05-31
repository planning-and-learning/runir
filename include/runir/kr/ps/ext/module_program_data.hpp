#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_DATA_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_DATA_HPP_

#include "runir/kr/ps/ext/module_index.hpp"
#include "runir/kr/ps/ext/module_program_index.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ext::ModuleProgram>
{
    Index<runir::kr::ps::ext::ModuleProgram> index;
    Index<runir::kr::ps::ext::Module> entry_module;
    IndexList<runir::kr::ps::ext::Module> modules;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(entry_module);
        ygg::clear(modules);
    }

    auto cista_members() const noexcept { return std::tie(index, entry_module, modules); }
    auto identifying_members() const noexcept { return std::tie(entry_module, modules); }
};

}  // namespace ygg

#endif
