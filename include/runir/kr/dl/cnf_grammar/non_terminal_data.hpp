#ifndef RUNIR_CNF_GRAMMAR_NON_TERMINAL_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_NON_TERMINAL_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/indices.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
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
    Data(const std::string& name_) : index(), name(name_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(name);
    }

    auto cista_members() const noexcept { return std::tie(index, name); }
    auto identifying_members() const noexcept { return std::tie(name); }
};

}

#endif
