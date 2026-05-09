#ifndef RUNIR_CNF_GRAMMAR_DERIVATION_RULE_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_DERIVATION_RULE_DATA_HPP_

#include "runir/knowledge_representation/dl/cnf_grammar/indices.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::cnf_grammar::DerivationRule<Category>>
{
    Index<runir::kr::dl::cnf_grammar::DerivationRule<Category>> index;
    Index<runir::kr::dl::cnf_grammar::NonTerminal<Category>> lhs;
    Index<runir::kr::dl::cnf_grammar::Constructor<Category>> rhs;

    Data() = default;
    Data(Index<runir::kr::dl::cnf_grammar::NonTerminal<Category>> lhs_, Index<runir::kr::dl::cnf_grammar::Constructor<Category>> rhs_) :
        index(),
        lhs(lhs_),
        rhs(rhs_)
    {
    }

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
