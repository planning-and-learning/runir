#ifndef RUNIR_KR_DL_GRAMMAR_UNS_NUMERICAL_DATA_HPP_
#define RUNIR_KR_DL_GRAMMAR_UNS_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/grammar/numerical_data.hpp"
#include "runir/kr/dl/uns/declarations.hpp"

namespace ygg
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Numerical<Family, runir::kr::dl::NumericalConstantTag>> :
    runir::kr::dl::semantics::IdentifierData<runir::kr::dl::grammar::Numerical<Family, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>
{
    using Base = runir::kr::dl::semantics::IdentifierData<runir::kr::dl::grammar::Numerical<Family, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>;
    using Base::Base;
};

// Binary numerical operators over two numerical choices.
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalBinaryTag Tag>
struct Data<runir::kr::dl::grammar::Numerical<Family, Tag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Numerical<Family, Tag>,
                                         runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::NumericalTag>,
                                         runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Numerical<Family, Tag>,
                                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::NumericalTag>,
                                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
