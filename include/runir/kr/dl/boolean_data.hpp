#ifndef RUNIR_KR_DL_BOOLEAN_DATA_HPP_
#define RUNIR_KR_DL_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/query_data.hpp"
#include "runir/kr/dl/role_data.hpp"

#include <urd/elements/flat.hpp>

namespace urd
{

template<typename F, typename Tag>
    requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
struct data<runir::kr::dl::Boolean<F, Tag>> : runir::kr::dl::PredicateData<runir::kr::dl::Boolean<F, Tag>, runir::kr::dl::TagFactKind<Tag>>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::Boolean<F, Tag>, runir::kr::dl::TagFactKind<Tag>>;
    using Base::Base;
};

template<typename F>
struct data<runir::kr::dl::Boolean<F, runir::kr::dl::NonemptyTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::Boolean<F, runir::kr::dl::NonemptyTag>, typename F::Argument>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::Boolean<F, runir::kr::dl::NonemptyTag>, typename F::Argument>;
    using Base::Base;
};

template<typename F>
struct data<runir::kr::dl::Boolean<F, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>> :
    runir::kr::dl::ReferenceData<runir::kr::dl::Boolean<F, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                 runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::ReferenceData<runir::kr::dl::Boolean<F, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                              runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

template<typename F, runir::kr::dl::ComparisonTag Tag>
struct data<runir::kr::dl::Boolean<F, Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Boolean<F, Tag>,
                              runir::kr::dl::operand_t<F, runir::kr::dl::comparison_operand_t<Tag>>,
                              runir::kr::dl::operand_t<F, runir::kr::dl::comparison_operand_t<Tag>>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Boolean<F, Tag>,
                                           runir::kr::dl::operand_t<F, runir::kr::dl::comparison_operand_t<Tag>>,
                                           runir::kr::dl::operand_t<F, runir::kr::dl::comparison_operand_t<Tag>>>;
    using Base::Base;
};

template<typename F>
struct data<runir::kr::dl::Boolean<F, runir::kr::dl::BooleanConstantTag>> :
    runir::kr::dl::IdentifierData<runir::kr::dl::Boolean<F, runir::kr::dl::BooleanConstantTag>, bool>
{
    using Base = runir::kr::dl::IdentifierData<runir::kr::dl::Boolean<F, runir::kr::dl::BooleanConstantTag>, bool>;
    using Base::Base;
};

// Binary logical operators (and/or): a Boolean over two Boolean operands.
template<typename F, runir::kr::dl::LogicalBinaryTag Tag>
struct data<runir::kr::dl::Boolean<F, Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Boolean<F, Tag>, runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag>, runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::
        BinaryData<runir::kr::dl::Boolean<F, Tag>, runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag>, runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

// Unary logical operator (not): a Boolean over one Boolean operand.
template<typename F>
struct data<runir::kr::dl::Boolean<F, runir::kr::dl::NotTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::Boolean<F, runir::kr::dl::NotTag>, runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::Boolean<F, runir::kr::dl::NotTag>, runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

}  // namespace urd

#endif
