#ifndef RUNIR_KR_PS_EXT_MEMORY_TRANSITION_DATA_HPP_
#define RUNIR_KR_PS_EXT_MEMORY_TRANSITION_DATA_HPP_

#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/memory_transition_index.hpp"
#include "runir/kr/ps/ext/rule_variant_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
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
        tyr::clear(index);
        tyr::clear(source);
        tyr::clear(target);
        tyr::clear(rules);
    }

    auto cista_members() const noexcept { return std::tie(index, source, target, rules); }
    auto identifying_members() const noexcept { return std::tie(source, target, rules); }
};

}  // namespace tyr

#endif
