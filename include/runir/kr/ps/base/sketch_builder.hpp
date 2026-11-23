#ifndef RUNIR_KR_PS_BASE_SKETCH_BUILDER_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_BUILDER_HPP_

#include "runir/kr/ps/base/sketch_data.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Sketch: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::base::Sketch, C>
{
    urd::key<runir::kr::ps::base::Sketch> index;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>>, C> boolean_features;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>>, C> numerical_features;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>>, C> rules;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"boolean_features", &builder::boolean_features>,
                           urd::field<"numerical_features", &builder::numerical_features>,
                           urd::field<"rules", &builder::rules>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::boolean_features, &builder::numerical_features, &builder::rules> {}; }
};

}  // namespace urd

#endif
