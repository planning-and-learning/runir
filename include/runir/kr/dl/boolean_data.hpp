#ifndef RUNIR_KR_DL_BOOLEAN_DATA_HPP_
#define RUNIR_KR_DL_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/query_data.hpp"
#include "runir/kr/dl/role_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>

namespace urd
{

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::Boolean<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::Boolean<runir::kr::dl::AtomicGoalTag<T>>> : runir::kr::dl::PredicateData<runir::kr::dl::Boolean<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::Boolean<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::Boolean<runir::kr::dl::NonemptyTag>>
{
    using ConstructorVariant = urd::index_variant<runir::kr::dl::DlArgumentTypes>;
    using Arg = ConstructorVariant;

    urd::index<runir::kr::dl::Boolean<runir::kr::dl::NonemptyTag>> index;
    ConstructorVariant arg;

    data() = default;
    explicit data(ConstructorVariant arg_) : index(), arg(std::move(arg_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg> {}; }
};

template<>
struct data<runir::kr::dl::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>> :
    runir::kr::dl::ReferenceData<runir::kr::dl::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                 runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::ReferenceData<runir::kr::dl::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                              runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

template<runir::kr::dl::ComparisonTag Tag>
struct data<runir::kr::dl::Boolean<Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Boolean<Tag>,
                              runir::kr::dl::Constructor<runir::kr::dl::comparison_operand_t<Tag>>,
                              runir::kr::dl::Constructor<runir::kr::dl::comparison_operand_t<Tag>>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Boolean<Tag>,
                                           runir::kr::dl::Constructor<runir::kr::dl::comparison_operand_t<Tag>>,
                                           runir::kr::dl::Constructor<runir::kr::dl::comparison_operand_t<Tag>>>;
    using Base::Base;
};

// Boolean constant.
template<>
struct data<runir::kr::dl::Boolean<runir::kr::dl::BooleanConstantTag>> :
    runir::kr::dl::IdentifierData<runir::kr::dl::Boolean<runir::kr::dl::BooleanConstantTag>, bool>
{
    using Base = runir::kr::dl::IdentifierData<runir::kr::dl::Boolean<runir::kr::dl::BooleanConstantTag>, bool>;
    using Base::Base;
};

// Binary logical operators (and/or): a Boolean over two Boolean operands.
template<runir::kr::dl::LogicalBinaryTag Tag>
struct data<runir::kr::dl::Boolean<Tag>> :
    runir::kr::dl::
        BinaryData<runir::kr::dl::Boolean<Tag>, runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>, runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::
        BinaryData<runir::kr::dl::Boolean<Tag>, runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>, runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

// Unary logical operator (not): a Boolean over one Boolean operand.
template<>
struct data<runir::kr::dl::Boolean<runir::kr::dl::NotTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::Boolean<runir::kr::dl::NotTag>, runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::Boolean<runir::kr::dl::NotTag>, runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

}  // namespace urd

#endif
