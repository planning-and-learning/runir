#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_DATA_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_DATA_HPP_

#include "runir/grammar/indices.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <variant>

namespace tyr
{

template<runir::CategoryTag Category>
struct Data<runir::grammar::ConstructorOrNonTerminal<Category>>
{
    using Variant = std::variant<Index<runir::grammar::Constructor<Category>>, Index<runir::grammar::NonTerminal<Category>>>;

    Index<runir::grammar::ConstructorOrNonTerminal<Category>> index;
    Variant value;

    Data() = default;
    Data(Variant value_) : index(), value(value_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        value = Variant();
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}

#endif
