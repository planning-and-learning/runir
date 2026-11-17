#ifndef RUNIR_KR_PS_MODULE_SYMBOL_DATA_HPP_
#define RUNIR_KR_PS_MODULE_SYMBOL_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

template<runir::kr::ps::ProgramFamilyTag Family>
struct data<runir::kr::ps::ModuleSymbol<Family>>
{
    urd::index<runir::kr::ps::ModuleSymbol<Family>> index;
    urd::flat_string name;

    data() = default;

    data(urd::flat_string name_) : index(), name(std::move(name_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"name", &data::name>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::name> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::name> {}; }
};

}  // namespace urd

#endif
