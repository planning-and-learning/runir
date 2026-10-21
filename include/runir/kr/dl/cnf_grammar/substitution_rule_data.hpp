#ifndef RUNIR_CNF_GRAMMAR_SUBSTITUTION_RULE_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_SUBSTITUTION_RULE_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>>
{
    Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>> index;
    Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>> lhs;
    Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>> rhs;

    Data() = default;
    Data(Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>> lhs_, Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>> rhs_) :
        index(),
        lhs(std::move(lhs_)),
        rhs(std::move(rhs_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>>, C> lhs_,
         ::ygg::View<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>>, C> rhs_) :
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

}

#endif
