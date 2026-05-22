#ifndef RUNIR_KR_MPG_DFA_DATA_HPP_
#define RUNIR_KR_MPG_DFA_DATA_HPP_

#include "runir/kr/mpg/automata_state_index.hpp"
#include "runir/kr/mpg/dfa_index.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::mpg::DFA>
{
    Index<runir::kr::mpg::DFA> index;
    ::cista::offset::string name;
    Index<runir::kr::mpg::AutomataState> initial_state;
    IndexList<runir::kr::mpg::AutomataState> states;

    Data() = default;
    Data(::cista::offset::string name_) : index(), name(std::move(name_)) {}
    Data(const std::string& name_) : index(), name(name_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(name);
        tyr::clear(initial_state);
        tyr::clear(states);
    }

    auto cista_members() const noexcept { return std::tie(index, name, initial_state, states); }
    auto identifying_members() const noexcept { return std::tie(name, initial_state, states); }
};

}  // namespace tyr

#endif
