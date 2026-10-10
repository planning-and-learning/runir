#ifndef RUNIR_CNF_GRAMMAR_DERIVATION_RULE_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_DERIVATION_RULE_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <tuple>
#include <utility>
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
        lhs(std::move(lhs_)),
        rhs(std::move(rhs_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>>, C> lhs_,
         ::ygg::View<Index<runir::kr::dl::cnf_grammar::Constructor<Family, Category>>, C> rhs_) :
        index(),
        lhs(),
        rhs()
    {
        set(lhs_, lhs);
        set(rhs_, rhs);
    }

    auto cista_members() noexcept { return std::tie(index, lhs, rhs); }
    auto cista_members() const noexcept { return std::tie(index, lhs, rhs); }
    auto identifying_members() const noexcept { return std::tie(lhs, rhs); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
