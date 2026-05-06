#ifndef RUNIR_GRAMMAR_NON_TERMINAL_DATA_HPP_
#define RUNIR_GRAMMAR_NON_TERMINAL_DATA_HPP_

#include "runir/grammar/indices.hpp"

#include <string>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<runir::CategoryTag Category>
struct Data<runir::grammar::NonTerminal<Category>>
{
    Index<runir::grammar::NonTerminal<Category>> index;
    std::string name;

    Data() = default;
    Data(std::string name_) : index(), name(std::move(name_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(name);
    }

    auto cista_members() const noexcept { return std::tie(index, name); }
    auto identifying_members() const noexcept { return std::tie(name); }
};

}

#endif
