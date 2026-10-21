#ifndef RUNIR_CNF_GRAMMAR_NON_TERMINAL_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_NON_TERMINAL_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>>
{
    Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>> index;
    ::cista::offset::string name;

    Data() = default;
    Data(::cista::offset::string name_) : index(), name(std::move(name_)) {}

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::name> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::name> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
    }
};

}

#endif
