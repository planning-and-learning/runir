#ifndef RUNIR_KR_DL_UNS_BOOLEAN_DATA_HPP_
#define RUNIR_KR_DL_UNS_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/boolean_data.hpp"
#include "runir/kr/dl/uns/declarations.hpp"

namespace ygg
{

// Comparison operators: a Boolean over two operands of the comparison's operand category.
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::ComparisonTag Tag>
struct Data<runir::kr::dl::Boolean<Family, Tag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Boolean<Family, Tag>,
                                         runir::kr::dl::Constructor<Family, runir::kr::dl::comparison_operand_t<Tag>>,
                                         runir::kr::dl::Constructor<Family, runir::kr::dl::comparison_operand_t<Tag>>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Boolean<Family, Tag>,
                                                      runir::kr::dl::Constructor<Family, runir::kr::dl::comparison_operand_t<Tag>>,
                                                      runir::kr::dl::Constructor<Family, runir::kr::dl::comparison_operand_t<Tag>>>;
    using Base::Base;
};

// Boolean constant.
template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Boolean<Family, runir::kr::dl::BooleanConstantTag>> :
    runir::kr::dl::semantics::IdentifierData<runir::kr::dl::Boolean<Family, runir::kr::dl::BooleanConstantTag>, bool>
{
    using Base = runir::kr::dl::semantics::IdentifierData<runir::kr::dl::Boolean<Family, runir::kr::dl::BooleanConstantTag>, bool>;
    using Base::Base;
};

}  // namespace ygg

#endif
