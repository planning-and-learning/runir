#ifndef RUNIR_KR_DL_EXT_NUMERICAL_DATA_HPP_
#define RUNIR_KR_DL_EXT_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/kr/dl/numerical_data.hpp"

namespace tyr
{

template<>
struct Data<runir::kr::dl::Numerical<runir::kr::dl::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> :
    runir::kr::dl::semantics::ArgumentData<runir::kr::dl::Numerical<runir::kr::dl::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                           runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>
{
    using Base =
        runir::kr::dl::semantics::ArgumentData<runir::kr::dl::Numerical<runir::kr::dl::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                               runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
