#ifndef RUNIR_KR_PS_UNS_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_UNS_CANONICALIZATION_HPP_

#include "runir/kr/ps/uns/declarations.hpp"
#include "runir/kr/ps/uns/rule_data.hpp"
#include "runir/kr/ps/uns/sketch_data.hpp"
#include "runir/kr/ps/canonicalization.hpp"

#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::kr::ps::uns
{

inline bool is_canonical(const ygg::Data<Rule>& data) noexcept
{
    return ygg::is_canonical(data.conditions) && ygg::is_canonical(data.effects);
}

inline bool is_canonical(const ygg::Data<Sketch>& data) noexcept { return ygg::is_canonical(data.rules); }

inline void canonicalize(ygg::Data<Rule>& data)
{
    ygg::canonicalize(data.conditions);
    ygg::canonicalize(data.effects);
}

inline void canonicalize(ygg::Data<Sketch>& data) { ygg::canonicalize(data.rules); }

}  // namespace runir::kr::ps::uns

#endif
