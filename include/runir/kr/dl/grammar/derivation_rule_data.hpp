#ifndef RUNIR_GRAMMAR_DERIVATION_RULE_DATA_HPP_
#define RUNIR_GRAMMAR_DERIVATION_RULE_DATA_HPP_

#include "runir/kr/dl/grammar/constructor_or_non_terminal_data.hpp"
#include "runir/kr/dl/grammar/derivation_rule_index.hpp"

#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::grammar::DerivationRule<Family, Category>>
{
    Index<runir::kr::dl::grammar::DerivationRule<Family, Category>> index;
    Index<runir::kr::dl::grammar::NonTerminal<Family, Category>> lhs;
    IndexList<runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, Category>> rhs;
    Data() = default;
    Data(Index<runir::kr::dl::grammar::NonTerminal<Family, Category>> lhs_,
         IndexList<runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, Category>> rhs_) :
        index(),
        lhs(lhs_),
        rhs(std::move(rhs_))
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

}  // namespace ygg

#endif
