#ifndef RUNIR_CNF_GRAMMAR_SUBSTITUTION_RULE_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_SUBSTITUTION_RULE_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

template<runir::kr::dl::CategoryTag Category>
struct data<runir::kr::dl::cnf_grammar::SubstitutionRule<Category>>
{
    urd::index<runir::kr::dl::cnf_grammar::SubstitutionRule<Category>> index;
    urd::index<runir::kr::dl::cnf_grammar::NonTerminal<Category>> lhs;
    urd::index<runir::kr::dl::cnf_grammar::NonTerminal<Category>> rhs;

    data() = default;
    data(urd::index<runir::kr::dl::cnf_grammar::NonTerminal<Category>> lhs_, urd::index<runir::kr::dl::cnf_grammar::NonTerminal<Category>> rhs_) :
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
