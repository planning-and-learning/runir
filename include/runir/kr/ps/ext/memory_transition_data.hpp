#ifndef RUNIR_KR_PS_EXT_MEMORY_TRANSITION_DATA_HPP_
#define RUNIR_KR_PS_EXT_MEMORY_TRANSITION_DATA_HPP_

#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/memory_transition_index.hpp"
#include "runir/kr/ps/ext/rule_variant_index.hpp"

#include <cista/containers/string.h>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ext::MemoryTransition>
{
    Index<runir::kr::ps::ext::MemoryTransition> index;
    Index<runir::kr::ps::ext::MemoryState> source;
    Index<runir::kr::ps::ext::MemoryState> target;
    ::cista::offset::string symbol;
    ::cista::offset::string description;
    IndexList<runir::kr::ps::ext::RuleVariant> rules;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(source);
        ygg::clear(target);
        ygg::clear(symbol);
        ygg::clear(description);
        ygg::clear(rules);
    }

    auto cista_members() const noexcept { return std::tie(index, source, target, symbol, description, rules); }
    auto identifying_members() const noexcept { return std::tie(source, target, symbol, description, rules); }
};

}  // namespace ygg

#endif
