#ifndef RUNIR_KR_MPG_MODULE_DATA_HPP_
#define RUNIR_KR_MPG_MODULE_DATA_HPP_

#include "runir/kr/gp/rule_index.hpp"
#include "runir/kr/mpg/automata_state_index.hpp"
#include "runir/kr/mpg/dfa_index.hpp"
#include "runir/kr/mpg/module_index.hpp"
#include "runir/kr/mpg/register_index.hpp"
#include "runir/kr/mpg/rule_data.hpp"
#include "runir/kr/mpg/rule_index.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::mpg::Module>
{
    Index<runir::kr::mpg::Module> index;
    ::cista::offset::string name;
    Index<runir::kr::mpg::DFA> dfa;
    IndexList<runir::kr::mpg::Register> registers;
    IndexList<runir::kr::mpg::AutomataState> automata_states;
    IndexList<runir::kr::gp::Rule> rules;
    IndexList<runir::kr::mpg::Rule<runir::kr::mpg::LoadTag>> load_rules;
    IndexList<runir::kr::mpg::Rule<runir::kr::mpg::DoTag>> do_rules;
    IndexList<runir::kr::mpg::Rule<runir::kr::mpg::CallTag>> call_rules;

    Data() = default;
    Data(::cista::offset::string name_) : index(), name(std::move(name_)) {}
    Data(const std::string& name_) : index(), name(name_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(name);
        tyr::clear(dfa);
        tyr::clear(registers);
        tyr::clear(automata_states);
        tyr::clear(rules);
        tyr::clear(load_rules);
        tyr::clear(do_rules);
        tyr::clear(call_rules);
    }

    auto cista_members() const noexcept { return std::tie(index, name, dfa, registers, automata_states, rules, load_rules, do_rules, call_rules); }
    auto identifying_members() const noexcept { return std::tie(name, dfa, registers, automata_states, rules, load_rules, do_rules, call_rules); }
};

}  // namespace tyr

#endif
