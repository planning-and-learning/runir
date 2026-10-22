#ifndef RUNIR_GRAMMAR_DERIVATION_RULE_DATA_HPP_
#define RUNIR_GRAMMAR_DERIVATION_RULE_DATA_HPP_

#include "runir/kr/dl/grammar/constructor_or_non_terminal_data.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
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
        lhs(std::move(lhs_)),
        rhs(std::move(rhs_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::grammar::NonTerminal<Category>>, C> lhs_,
         const std::vector<::ygg::View<Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>>, C>>& rhs_) :
        index(),
        lhs(),
        rhs()
    {
        set(lhs_, lhs);
        set(rhs_, rhs);
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::lhs, &Data::rhs> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::lhs, &Data::rhs> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
    }
};

}  // namespace ygg

#endif
