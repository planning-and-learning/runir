#ifndef RUNIR_KR_DL_BOOLEAN_DATA_HPP_
#define RUNIR_KR_DL_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/query_data.hpp"
#include "runir/kr/dl/role_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Boolean<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Boolean<runir::kr::dl::AtomicGoalTag<T>>> : runir::kr::dl::PredicateData<runir::kr::dl::Boolean<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::Boolean<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Boolean<runir::kr::dl::NonemptyTag>>
{
    using ConstructorVariant = ::ygg::IndexVariant<DlArgumentTypes>;
    using Arg = ConstructorVariant;

    Index<runir::kr::dl::Boolean<runir::kr::dl::NonemptyTag>> index;
    ConstructorVariant arg;

    Data() = default;
    explicit Data(ConstructorVariant arg_) : index(), arg(std::move(arg_)) {}
    template<typename C>
    using ViewVariant = std::variant<::ygg::View<Index<DlConcept>, C>, ::ygg::View<Index<DlRole>, C>, ::ygg::View<Index<runir::kr::dl::Query<>>, C>>;
    template<typename C>
    explicit Data(ViewVariant<C> arg_) :
        index(),
        arg(std::visit([](const auto& view) -> ConstructorVariant { return ConstructorVariant(view.get_index()); }, arg_))
    {
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::arg> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::arg> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::dl::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>> :
    runir::kr::dl::ReferenceData<runir::kr::dl::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                 runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::ReferenceData<runir::kr::dl::Boolean<runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>,
                                              runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

template<runir::kr::dl::ComparisonTag Tag>
struct Data<runir::kr::dl::Boolean<Tag>> :
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
struct Data<runir::kr::dl::Boolean<runir::kr::dl::BooleanConstantTag>> :
    runir::kr::dl::IdentifierData<runir::kr::dl::Boolean<runir::kr::dl::BooleanConstantTag>, bool>
{
    using Base = runir::kr::dl::IdentifierData<runir::kr::dl::Boolean<runir::kr::dl::BooleanConstantTag>, bool>;
    using Base::Base;
};

// Binary logical operators (and/or): a Boolean over two Boolean operands.
template<runir::kr::dl::LogicalBinaryTag Tag>
struct Data<runir::kr::dl::Boolean<Tag>> :
    runir::kr::dl::
        BinaryData<runir::kr::dl::Boolean<Tag>, runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>, runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::
        BinaryData<runir::kr::dl::Boolean<Tag>, runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>, runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

// Unary logical operator (not): a Boolean over one Boolean operand.
template<>
struct Data<runir::kr::dl::Boolean<runir::kr::dl::NotTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::Boolean<runir::kr::dl::NotTag>, runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::Boolean<runir::kr::dl::NotTag>, runir::kr::dl::Constructor<runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
