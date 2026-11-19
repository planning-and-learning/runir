#ifndef RUNIR_KR_PS_RULE_VARIANT_BUILDER_HPP_
#define RUNIR_KR_PS_RULE_VARIANT_BUILDER_HPP_

#include "runir/kr/ps/rule_variant_data.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Rule: its record's members, each in its built form (urd::built_t).
template<runir::kr::ps::ProgramFamilyTag Family, typename C>
struct builder<runir::kr::ps::Rule<Family>, C>
{
    urd::index<runir::kr::ps::Rule<Family>> index;
    urd::built_t<urd::flat_string, C> symbol;
    urd::built_t<runir::kr::ps::RuleVariant<Family>, C> variant;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"symbol", &builder::symbol>, urd::field<"variant", &builder::variant>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::symbol, &builder::variant> {}; }
};

}  // namespace urd

#endif
