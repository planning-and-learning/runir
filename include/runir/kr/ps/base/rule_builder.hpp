#ifndef RUNIR_KR_PS_BASE_RULE_BUILDER_HPP_
#define RUNIR_KR_PS_BASE_RULE_BUILDER_HPP_

#include "runir/kr/ps/base/rule_data.hpp"
#include "runir/kr/ps/base/rule_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Rule: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>, C>
{
    urd::index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>> index;
    urd::built_t<urd::flat_string, C> symbol;
    urd::built_t<runir::kr::ps::ConditionList<runir::kr::BaseFamilyTag>, C> conditions;
    urd::built_t<runir::kr::ps::EffectList<runir::kr::BaseFamilyTag>, C> effects;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"symbol", &builder::symbol>, urd::field<"conditions", &builder::conditions>, urd::field<"effects", &builder::effects>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::symbol, &builder::conditions, &builder::effects> {}; }
};

}  // namespace urd

#endif
