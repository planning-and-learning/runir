#ifndef RUNIR_KR_PS_EXT_PROGRAM_DATA_HPP_
#define RUNIR_KR_PS_EXT_PROGRAM_DATA_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>

namespace urd
{

template<>
struct data<runir::kr::ps::ext::Program>
{
    urd::index<runir::kr::ps::ext::Program> index;
    urd::index<runir::kr::ps::ext::Module> entry_module;
    urd::flat_vector<urd::index<runir::kr::ps::ext::Module>> modules;

    data() = default;
    data(urd::index<runir::kr::ps::ext::Module> entry_module_, urd::flat_vector<urd::index<runir::kr::ps::ext::Module>> modules_) :
        index(),
        entry_module(entry_module_),
        modules(std::move(modules_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::entry_module, &data::modules> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::entry_module, &data::modules> {}; }
};

}  // namespace urd

#endif
