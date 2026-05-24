#ifndef RUNIR_KR_PS_EXT_DL_CONDITION_DATA_HPP_
#define RUNIR_KR_PS_EXT_DL_CONDITION_DATA_HPP_

#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/ext/condition_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<typename FeatureTag, typename ObservationTag>
struct Data<runir::kr::ps::ext::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>
{
    Index<runir::kr::ps::ext::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>> index;
    Index<runir::kr::ps::ext::Feature<FeatureTag>> feature;

    Data() = default;
    Data(Index<runir::kr::ps::ext::Feature<FeatureTag>> feature_) : index(), feature(feature_) {}

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
