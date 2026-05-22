#ifndef RUNIR_KR_MPG_POLICY_DATA_HPP_
#define RUNIR_KR_MPG_POLICY_DATA_HPP_

#include "runir/kr/mpg/module_index.hpp"
#include "runir/kr/mpg/policy_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::mpg::Policy>
{
    Index<runir::kr::mpg::Policy> index;
    Index<runir::kr::mpg::Module> entry_module;
    IndexList<runir::kr::mpg::Module> modules;

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
