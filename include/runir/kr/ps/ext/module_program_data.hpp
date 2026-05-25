#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_DATA_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_DATA_HPP_

#include "runir/kr/ps/ext/module_index.hpp"
#include "runir/kr/ps/ext/module_program_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::ps::ext::ModuleProgram>
{
    Index<runir::kr::ps::ext::ModuleProgram> index;
    Index<runir::kr::ps::ext::Module> entry_module;
    IndexList<runir::kr::ps::ext::Module> modules;

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(entry_module);
        tyr::clear(modules);
    }

    auto cista_members() const noexcept { return std::tie(index, entry_module, modules); }
    auto identifying_members() const noexcept { return std::tie(entry_module, modules); }
};

}  // namespace tyr

#endif
