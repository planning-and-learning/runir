#ifndef RUNIR_CNF_GRAMMAR_NON_TERMINAL_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_NON_TERMINAL_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <urd/elements/flat.hpp>

namespace urd
{

template<runir::kr::dl::CategoryTag Category>
struct data<runir::kr::dl::cnf_grammar::NonTerminal<Category>>
{
    urd::index<runir::kr::dl::cnf_grammar::NonTerminal<Category>> index;
    urd::flat_string name;

    data() = default;
    data(urd::flat_string name_) : index(), name(std::move(name_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::name> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::name> {}; }
};

}

#endif
