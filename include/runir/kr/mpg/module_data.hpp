#ifndef RUNIR_KR_MPG_MODULE_DATA_HPP_
#define RUNIR_KR_MPG_MODULE_DATA_HPP_

#include "runir/kr/mpg/memory_state_index.hpp"
#include "runir/kr/mpg/module_index.hpp"
#include "runir/kr/mpg/register_index.hpp"
#include "runir/kr/mpg/rule_data.hpp"
#include "runir/kr/mpg/rule_index.hpp"
#include "runir/kr/mpg/rule_variant_index.hpp"

#include <cista/containers/string.h>
#include <cista/containers/vector.h>
#include <string>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace runir::kr::mpg
{

struct MemoryTransition
{
    tyr::Index<MemoryState> source;
    tyr::Index<MemoryState> target;
    tyr::IndexList<RuleVariant> rules;

    void clear() noexcept
    {
        tyr::clear(source);
        tyr::clear(target);
        tyr::clear(rules);
    }

    auto cista_members() const noexcept { return std::tie(source, target, rules); }
    auto identifying_members() const noexcept { return std::tie(source, target, rules); }
};

}  // namespace runir::kr::mpg

namespace tyr
{

template<>
struct Data<runir::kr::mpg::Module>
{
    using RuleGraph = ::cista::offset::vector<runir::kr::mpg::MemoryTransition>;

    Index<runir::kr::mpg::Module> index;
    ::cista::offset::string name;
    IndexList<runir::kr::mpg::Register> registers;
    Index<runir::kr::mpg::MemoryState> entry_memory_state;
    IndexList<runir::kr::mpg::MemoryState> memory_states;
    RuleGraph memory_transitions;
    Data() = default;
    Data(::cista::offset::string name_) : index(), name(std::move(name_)) {}
    Data(const std::string& name_) : index(), name(name_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(name);
        tyr::clear(registers);
        tyr::clear(entry_memory_state);
        tyr::clear(memory_states);
        tyr::clear(memory_transitions);
    }

    auto cista_members() const noexcept { return std::tie(index, name, registers, entry_memory_state, memory_states, memory_transitions); }
    auto identifying_members() const noexcept { return std::tie(name, registers, entry_memory_state, memory_states, memory_transitions); }
};

}  // namespace tyr

#endif
