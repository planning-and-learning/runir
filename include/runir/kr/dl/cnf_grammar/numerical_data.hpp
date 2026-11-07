#ifndef RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/data_helpers.hpp"

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
struct data<runir::kr::dl::cnf_grammar::Numerical<runir::kr::dl::CountTag>>
{
    using Arg = urd::index_variant<boost::mp11::mp_transform<runir::kr::dl::cnf_grammar::NonTerminal, runir::kr::dl::ConceptOrRoleTags>>;

    urd::index<runir::kr::dl::cnf_grammar::Numerical<runir::kr::dl::CountTag>> index;
    Arg arg;

    data() = default;
    explicit data(Arg arg_) : index(), arg(std::move(arg_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::arg> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::arg> {}; }
};

template<>
struct data<runir::kr::dl::cnf_grammar::Numerical<runir::kr::dl::DistanceTag>> :
    runir::kr::dl::TernaryData<runir::kr::dl::cnf_grammar::Numerical<runir::kr::dl::DistanceTag>,
                               runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>,
                               runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>,
                               runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::TernaryData<runir::kr::dl::cnf_grammar::Numerical<runir::kr::dl::DistanceTag>,
                                            runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>,
                                            runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>,
                                            runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace urd

#endif
