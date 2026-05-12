#ifndef RUNIR_KR_GP_EFFECT_DATA_HPP_
#define RUNIR_KR_GP_EFFECT_DATA_HPP_

#include "runir/kr/gp/dl/declarations.hpp"
#include "runir/kr/gp/dl/effect_data.hpp"
#include "runir/kr/gp/effect_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::gp::EffectVariant>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::gp::ConcreteEffectVariant<runir::kr::DlTag>>>;

    Index<runir::kr::gp::EffectVariant> index;
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
struct Data<runir::kr::gp::ConcreteEffectVariant<runir::kr::DlTag>>
{
    using Variant =
        ::cista::offset::variant<Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, runir::kr::gp::dl::BooleanFeature, runir::kr::gp::dl::BecomesTrue>>,
                                 Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, runir::kr::gp::dl::BooleanFeature, runir::kr::gp::dl::BecomesFalse>>,
                                 Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, runir::kr::gp::dl::BooleanFeature, runir::kr::gp::dl::Unchanged>>,
                                 Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, runir::kr::gp::dl::NumericalFeature, runir::kr::gp::dl::Increases>>,
                                 Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, runir::kr::gp::dl::NumericalFeature, runir::kr::gp::dl::Decreases>>,
                                 Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, runir::kr::gp::dl::NumericalFeature, runir::kr::gp::dl::Unchanged>>>;

    Index<runir::kr::gp::ConcreteEffectVariant<runir::kr::DlTag>> index;
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
