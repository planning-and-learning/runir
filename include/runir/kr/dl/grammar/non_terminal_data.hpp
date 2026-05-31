#ifndef RUNIR_GRAMMAR_NON_TERMINAL_DATA_HPP_
#define RUNIR_GRAMMAR_NON_TERMINAL_DATA_HPP_

#include "runir/kr/dl/grammar/indices.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <utility>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::grammar::NonTerminal<Family, Category>>
{
    Index<runir::kr::dl::grammar::NonTerminal<Family, Category>> index;
    ::cista::offset::string name;
    Data() = default;
    explicit Data(::cista::offset::string name_) : index(), name(std::move(name_)) {}
    explicit Data(const std::string& name_) : index(), name(name_) {}
    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(name);
    }
    auto cista_members() const noexcept { return std::tie(index, name); }
    auto identifying_members() const noexcept { return std::tie(name); }
};

}  // namespace ygg

#endif
