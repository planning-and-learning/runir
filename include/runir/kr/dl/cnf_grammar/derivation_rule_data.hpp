#ifndef RUNIR_CNF_GRAMMAR_DERIVATION_RULE_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_DERIVATION_RULE_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

template<runir::kr::dl::CategoryTag Category>
struct data<runir::kr::dl::cnf_grammar::DerivationRule<Category>>
{
    urd::index<runir::kr::dl::cnf_grammar::DerivationRule<Category>> index;
    urd::index<runir::kr::dl::cnf_grammar::NonTerminal<Category>> lhs;
    runir::kr::dl::cnf_grammar::Constructor<Category> rhs;

    data() = default;
    data(urd::index<runir::kr::dl::cnf_grammar::NonTerminal<Category>> lhs_, runir::kr::dl::cnf_grammar::Constructor<Category> rhs_) :
        index(),
        lhs(std::move(lhs_)),
        rhs(std::move(rhs_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::lhs, &data::rhs> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::lhs, &data::rhs> {}; }
};

}

#endif
