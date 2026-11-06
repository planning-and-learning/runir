#ifndef RUNIR_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_GRAMMAR_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/grammar/boolean_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>
#include <yggdrasil/core/config.hpp>

namespace urd
{

template<>
struct data<runir::kr::dl::grammar::Numerical<runir::kr::dl::CountTag>>
{
    using Arg = boost::mp11::mp_apply<::cista::offset::variant, runir::kr::dl::grammar::GrammarArgumentTypes>;
    urd::index<runir::kr::dl::grammar::Numerical<runir::kr::dl::CountTag>> index;
    Arg arg;
    data() = default;
    explicit data(Arg arg_) : index(), arg(std::move(arg_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg> {}; }
};

template<>
struct data<runir::kr::dl::grammar::Numerical<runir::kr::dl::DistanceTag>> :
    runir::kr::dl::TernaryData<runir::kr::dl::grammar::Numerical<runir::kr::dl::DistanceTag>, runir::kr::dl::grammar::GrammarConceptChoice, runir::kr::dl::grammar::GrammarRoleChoice, runir::kr::dl::grammar::GrammarConceptChoice>
{
    using Base = runir::kr::dl::
        TernaryData<runir::kr::dl::grammar::Numerical<runir::kr::dl::DistanceTag>, runir::kr::dl::grammar::GrammarConceptChoice, runir::kr::dl::grammar::GrammarRoleChoice, runir::kr::dl::grammar::GrammarConceptChoice>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::grammar::Numerical<runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> :
    runir::kr::dl::ArgumentData<runir::kr::dl::grammar::Numerical<runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::ArgumentData<runir::kr::dl::grammar::Numerical<runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>,
                                             runir::kr::dl::ArgumentIdentifier<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::grammar::Numerical<runir::kr::dl::NumericalConstantTag>> :
    runir::kr::dl::IdentifierData<runir::kr::dl::grammar::Numerical<runir::kr::dl::NumericalConstantTag>, ygg::uint_t>
{
    using Base = runir::kr::dl::IdentifierData<runir::kr::dl::grammar::Numerical<runir::kr::dl::NumericalConstantTag>, ygg::uint_t>;
    using Base::Base;
};

template<runir::kr::dl::NumericalBinaryTag Tag>
struct data<runir::kr::dl::grammar::Numerical<Tag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::grammar::Numerical<Tag>,
                              runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::NumericalTag>,
                              runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::grammar::Numerical<Tag>,
                                           runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::NumericalTag>,
                                           runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

}  // namespace urd

#endif
