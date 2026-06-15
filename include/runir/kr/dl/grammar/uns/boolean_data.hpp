#ifndef RUNIR_KR_DL_GRAMMAR_UNS_BOOLEAN_DATA_HPP_
#define RUNIR_KR_DL_GRAMMAR_UNS_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/grammar/numerical_data.hpp"
#include "runir/kr/dl/uns/declarations.hpp"

namespace ygg
{

// Comparison operators over two operand-category choices (allowing non-terminals).
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::ComparisonTag Tag>
struct Data<runir::kr::dl::grammar::Boolean<Family, Tag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Boolean<Family, Tag>,
                                         runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>,
                                         runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Boolean<Family, Tag>,
                                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>,
                                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::BooleanConstantTag>> :
    runir::kr::dl::semantics::IdentifierData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::BooleanConstantTag>, bool>
{
    using Base = runir::kr::dl::semantics::IdentifierData<runir::kr::dl::grammar::Boolean<Family, runir::kr::dl::BooleanConstantTag>, bool>;
    using Base::Base;
};

}  // namespace ygg

#endif
