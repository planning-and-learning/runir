#ifndef RUNIR_KR_PS_DL_FEATURE_BUILDER_HPP_
#define RUNIR_KR_PS_DL_FEATURE_BUILDER_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/dl/feature_data.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of ConcreteFeature: its record's members, each in its built form (urd::built_t).
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
struct builder<runir::kr::ps::ConcreteFeature<Family, FeatureTag>, C>
{
    urd::key<runir::kr::ps::ConcreteFeature<Family, FeatureTag>> index;
    urd::built_t<urd::index<typename runir::kr::ps::dl::FeatureExpression<Family, FeatureTag>::Type>, ::runir::kr::dl::ConstructorRepository> feature;
    urd::built_t<urd::flat_string, C> symbol;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"feature", &builder::feature>, urd::field<"symbol", &builder::symbol>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::feature, &builder::symbol> {}; }
};

}  // namespace urd

#endif
