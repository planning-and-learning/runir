#ifndef RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_
#define RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/grammar/role_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>

namespace urd
{

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::grammar::Boolean<runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::grammar::Boolean<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::grammar::Boolean<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::grammar::Boolean<runir::kr::dl::NonemptyTag>>
{
    using Arg = boost::mp11::mp_apply<::cista::offset::variant, runir::kr::dl::grammar::GrammarArgumentTypes>;
    urd::index<runir::kr::dl::grammar::Boolean<runir::kr::dl::NonemptyTag>> index;
    Arg arg;
    data() = default;
    explicit data(Arg arg_) : index(), arg(std::move(arg_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg> {}; }
};

}  // namespace urd

#endif
