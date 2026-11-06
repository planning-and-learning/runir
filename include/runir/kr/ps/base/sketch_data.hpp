#ifndef RUNIR_KR_PS_BASE_SKETCH_DATA_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_DATA_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <tuple>
#include <utility>
#include <vector>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<runir::kr::ps::base::Sketch>
{
    urd::index<runir::kr::ps::base::Sketch> index;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>> boolean_features;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>> numerical_features;
    urd::flat_vector<urd::index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>> rules;

    data() = default;
    data(urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>> boolean_features_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>> numerical_features_,
         urd::flat_vector<urd::index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>> rules_) :
        index(),
        boolean_features(std::move(boolean_features_)),
        numerical_features(std::move(numerical_features_)),
        rules(std::move(rules_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::boolean_features, &data::numerical_features, &data::rules> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::boolean_features, &data::numerical_features, &data::rules> {}; }
};

}  // namespace urd

#endif
