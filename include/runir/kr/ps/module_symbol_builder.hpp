#ifndef RUNIR_KR_PS_MODULE_SYMBOL_BUILDER_HPP_
#define RUNIR_KR_PS_MODULE_SYMBOL_BUILDER_HPP_

#include "runir/kr/ps/module_symbol_data.hpp"
#include "runir/kr/ps/module_symbol_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of ModuleSymbol: its record's members, each in its built form (urd::built_t).
template<runir::kr::ps::ProgramFamilyTag Family, typename C>
struct builder<runir::kr::ps::ModuleSymbol<Family>, C>
{
    urd::key<runir::kr::ps::ModuleSymbol<Family>> index;
    urd::built_t<urd::flat_string, C> name;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"name", &builder::name>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::name> {}; }
};

}  // namespace urd

#endif
