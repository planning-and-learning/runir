#ifndef RUNIR_CNF_GRAMMAR_DERIVATION_RULE_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_DERIVATION_RULE_DATA_HPP_

#include "runir/cnf_grammar/indices.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<runir::CategoryTag Category>
struct Data<runir::cnf_grammar::DerivationRule<Category>>
{
    Index<runir::cnf_grammar::DerivationRule<Category>> index;
    Index<runir::cnf_grammar::NonTerminal<Category>> lhs;
    Index<runir::cnf_grammar::Constructor<Category>> rhs;

    Data() = default;
    Data(Index<runir::cnf_grammar::NonTerminal<Category>> lhs_, Index<runir::cnf_grammar::Constructor<Category>> rhs_) : index(), lhs(lhs_), rhs(rhs_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(lhs);
        tyr::clear(rhs);
    }

    auto cista_members() const noexcept { return std::tie(index, lhs, rhs); }
    auto identifying_members() const noexcept { return std::tie(lhs, rhs); }
};

}

#endif
