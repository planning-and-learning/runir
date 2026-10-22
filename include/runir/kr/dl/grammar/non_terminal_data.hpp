#ifndef RUNIR_GRAMMAR_NON_TERMINAL_DATA_HPP_
#define RUNIR_GRAMMAR_NON_TERMINAL_DATA_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::grammar::NonTerminal<Category>>
{
    Index<runir::kr::dl::grammar::NonTerminal<Category>> index;
    ::cista::offset::string name;

    Data() = default;
    explicit Data(::cista::offset::string name_) : index(), name(std::move(name_)) {}

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::name> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::name> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
    }
};

}  // namespace ygg

#endif
