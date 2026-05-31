#ifndef RUNIR_KR_DL_GRAMMAR_EXT_NUMERICAL_DATA_HPP_
#define RUNIR_KR_DL_GRAMMAR_EXT_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/kr/dl/grammar/numerical_data.hpp"

namespace ygg
{

template<>
struct Data<runir::kr::dl::grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> :
    runir::kr::dl::semantics::ArgumentData<runir::kr::dl::grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                           runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::semantics::ArgumentData<
        runir::kr::dl::grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
        runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
