#ifndef RUNIR_KR_DL_UNS_NUMERICAL_DATA_HPP_
#define RUNIR_KR_DL_UNS_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/numerical_data.hpp"
#include "runir/kr/dl/uns/declarations.hpp"

namespace ygg
{

// Nonnegative integer constant.
template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Numerical<Family, runir::kr::dl::NumericalConstantTag>> :
    runir::kr::dl::semantics::IdentifierData<runir::kr::dl::Numerical<Family, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>
{
    using Base = runir::kr::dl::semantics::IdentifierData<runir::kr::dl::Numerical<Family, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>;
    using Base::Base;
};

}  // namespace ygg

#endif
