#ifndef RUNIR_KR_GP_DL_CONDITION_DATA_HPP_
#define RUNIR_KR_GP_DL_CONDITION_DATA_HPP_

#include "runir/kr/gp/condition_index.hpp"
#include "runir/kr/gp/dl/declarations.hpp"
#include "runir/kr/gp/feature_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<typename FeatureTypeTag, typename ObservationTag>
struct Data<runir::kr::gp::Condition<runir::kr::DlTag, FeatureTypeTag, ObservationTag>>
{
    Index<runir::kr::gp::Condition<runir::kr::DlTag, FeatureTypeTag, ObservationTag>> index;
    Index<runir::kr::gp::FeatureTag<FeatureTypeTag>> feature;

    Data() = default;
    Data(Index<runir::kr::gp::FeatureTag<FeatureTypeTag>> feature_) : index(), feature(feature_) {}

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
