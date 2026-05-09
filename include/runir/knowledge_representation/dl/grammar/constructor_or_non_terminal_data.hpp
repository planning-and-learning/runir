#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_DATA_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_DATA_HPP_

#include "runir/knowledge_representation/dl/grammar/indices.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::dl::grammar::Constructor<Category>>, Index<runir::kr::dl::grammar::NonTerminal<Category>>>;

    Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>> index;
    Variant value;

    Data() = default;
    Data(Variant value_) : index(), value(value_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}

#endif
