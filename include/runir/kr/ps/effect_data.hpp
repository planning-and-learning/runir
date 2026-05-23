#ifndef RUNIR_KR_PS_EFFECT_DATA_HPP_
#define RUNIR_KR_PS_EFFECT_DATA_HPP_

#include "runir/kr/ps/base/dl/declarations.hpp"
#include "runir/kr/ps/base/dl/effect_data.hpp"
#include "runir/kr/ps/effect_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::ps::EffectVariant>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::DlTag>>>;

    Index<runir::kr::ps::EffectVariant> index;
    Variant value;

    Data() = default;
    Data(Variant value_) : index(), value(std::move(value_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<>
struct Data<runir::kr::ps::ConcreteEffectVariant<runir::kr::DlTag>>
{
    using Variant =
        ::cista::offset::variant<Index<runir::kr::ps::ConcreteEffect<runir::kr::DlTag, runir::kr::ps::base::dl::BooleanFeature, runir::kr::ps::base::dl::Positive>>,
                                 Index<runir::kr::ps::ConcreteEffect<runir::kr::DlTag, runir::kr::ps::base::dl::BooleanFeature, runir::kr::ps::base::dl::Negative>>,
                                 Index<runir::kr::ps::ConcreteEffect<runir::kr::DlTag, runir::kr::ps::base::dl::BooleanFeature, runir::kr::ps::base::dl::Unchanged>>,
                                 Index<runir::kr::ps::ConcreteEffect<runir::kr::DlTag, runir::kr::ps::base::dl::NumericalFeature, runir::kr::ps::base::dl::Increases>>,
                                 Index<runir::kr::ps::ConcreteEffect<runir::kr::DlTag, runir::kr::ps::base::dl::NumericalFeature, runir::kr::ps::base::dl::Decreases>>,
                                 Index<runir::kr::ps::ConcreteEffect<runir::kr::DlTag, runir::kr::ps::base::dl::NumericalFeature, runir::kr::ps::base::dl::Unchanged>>>;

    Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::DlTag>> index;
    Variant value;

    Data() = default;
    Data(Variant value_) : index(), value(std::move(value_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}  // namespace tyr

#endif
