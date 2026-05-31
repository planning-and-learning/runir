#ifndef RUNIR_KR_PS_EXT_MEMORY_TRANSITION_DATA_HPP_
#define RUNIR_KR_PS_EXT_MEMORY_TRANSITION_DATA_HPP_

#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/memory_transition_index.hpp"
#include "runir/kr/ps/ext/rule_variant_index.hpp"

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
    IndexList<runir::kr::ps::ext::RuleVariant> rules;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(source);
        ygg::clear(target);
        ygg::clear(rules);
    }

    auto cista_members() const noexcept { return std::tie(index, source, target, rules); }
    auto identifying_members() const noexcept { return std::tie(source, target, rules); }
};

}  // namespace ygg

#endif
