#ifndef RUNIR_CNF_GRAMMAR_DERIVATION_RULE_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_DERIVATION_RULE_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/indices.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::cnf_grammar::DerivationRule<Family, Category>>
{
    Index<runir::kr::dl::cnf_grammar::DerivationRule<Family, Category>> index;
    Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>> lhs;
    Index<runir::kr::dl::cnf_grammar::Constructor<Family, Category>> rhs;

    Data() = default;
    Data(Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>> lhs_, Index<runir::kr::dl::cnf_grammar::Constructor<Family, Category>> rhs_) :
        index(),
        lhs(lhs_),
        rhs(rhs_)
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(lhs);
        ygg::clear(rhs);
    }

    auto cista_members() const noexcept { return std::tie(index, lhs, rhs); }
    auto identifying_members() const noexcept { return std::tie(lhs, rhs); }
};

}

#endif
