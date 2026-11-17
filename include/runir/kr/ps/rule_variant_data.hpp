#ifndef RUNIR_KR_PS_RULE_VARIANT_DATA_HPP_
#define RUNIR_KR_PS_RULE_VARIANT_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <boost/mp11.hpp>
#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <string>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <utility>
#include <variant>

namespace runir::kr::ps
{

/// The concrete rule a rule variant of Family holds: one of the family's rule kinds.
template<runir::kr::ps::ProgramFamilyTag Family>
using RuleVariant = urd::index_variant<typename runir::kr::ps::FamilyTypes<Family>::ConcreteRuleTypes>;

}  // namespace runir::kr::ps

namespace urd
{

/// A program family's rule variant: a named rule of one of the family's rule kinds.
template<runir::kr::ps::ProgramFamilyTag Family>
struct data<runir::kr::ps::Rule<Family>>
{
    urd::index<runir::kr::ps::Rule<Family>> index;
    urd::flat_string symbol;
    runir::kr::ps::RuleVariant<Family> variant;

    data() = default;

    data(urd::flat_string symbol_, runir::kr::ps::RuleVariant<Family> variant_) : index(), symbol(std::move(symbol_)), variant(std::move(variant_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"symbol", &data::symbol>, urd::field<"variant", &data::variant>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::symbol, &data::variant> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::symbol, &data::variant> {}; }
};

}  // namespace urd

#endif
