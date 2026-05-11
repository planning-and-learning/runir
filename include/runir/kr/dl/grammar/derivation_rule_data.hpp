#ifndef RUNIR_GRAMMAR_DERIVATION_RULE_DATA_HPP_
#define RUNIR_GRAMMAR_DERIVATION_RULE_DATA_HPP_

#include "runir/kr/dl/grammar/indices.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::grammar::DerivationRule<Category>>
{
    Index<runir::kr::dl::grammar::DerivationRule<Category>> index;
    Index<runir::kr::dl::grammar::NonTerminal<Category>> lhs;
    IndexList<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>> rhs;

    Data() = default;
    Data(Index<runir::kr::dl::grammar::NonTerminal<Category>> lhs_, IndexList<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>> rhs_) :
        index(),
        lhs(lhs_),
        rhs(std::move(rhs_))
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
