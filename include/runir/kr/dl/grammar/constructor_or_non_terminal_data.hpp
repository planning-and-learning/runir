#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_DATA_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_DATA_HPP_

#include "runir/kr/dl/grammar/constructor_index.hpp"
#include "runir/kr/dl/grammar/non_terminal_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <utility>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, Category>>
{
    using Variant =
        ::cista::offset::variant<Index<runir::kr::dl::grammar::Constructor<Family, Category>>, Index<runir::kr::dl::grammar::NonTerminal<Family, Category>>>;
    Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, Category>> index;
    Variant value;
    Data() = default;
    explicit Data(Variant value_) : index(), value(std::move(value_)) {}
    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }
    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}  // namespace tyr

#endif
