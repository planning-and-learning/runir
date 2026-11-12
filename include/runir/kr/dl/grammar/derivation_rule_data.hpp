#ifndef RUNIR_GRAMMAR_DERIVATION_RULE_DATA_HPP_
#define RUNIR_GRAMMAR_DERIVATION_RULE_DATA_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <cista/containers/vector.h>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <utility>
#include <vector>

namespace urd
{

template<runir::kr::dl::CategoryTag Category>
struct data<runir::kr::dl::grammar::DerivationRule<Category>>
{
    urd::index<runir::kr::dl::grammar::DerivationRule<Category>> index;
    urd::index<runir::kr::dl::grammar::NonTerminal<Category>> lhs;
    urd::flat_vector<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>> rhs;
    data() = default;
    data(urd::index<runir::kr::dl::grammar::NonTerminal<Category>> lhs_, urd::flat_vector<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>> rhs_) :
        index(),
        lhs(std::move(lhs_)),
        rhs(std::move(rhs_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::lhs, &data::rhs> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::lhs, &data::rhs> {}; }
};

}  // namespace urd

#endif
