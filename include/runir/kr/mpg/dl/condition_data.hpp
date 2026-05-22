#ifndef RUNIR_KR_MPG_DL_CONDITION_DATA_HPP_
#define RUNIR_KR_MPG_DL_CONDITION_DATA_HPP_

#include "runir/kr/mpg/condition_index.hpp"
#include "runir/kr/mpg/dl/declarations.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<typename FeatureTag, typename ObservationTag>
struct Data<runir::kr::mpg::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>
{
    Index<runir::kr::mpg::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>> index;
    Index<runir::kr::mpg::Feature<FeatureTag>> feature;

    Data() = default;
    Data(Index<runir::kr::mpg::Feature<FeatureTag>> feature_) : index(), feature(feature_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(feature);
    }

    auto cista_members() const noexcept { return std::tie(index, feature); }
    auto identifying_members() const noexcept { return std::tie(feature); }
};

}  // namespace tyr

#endif
