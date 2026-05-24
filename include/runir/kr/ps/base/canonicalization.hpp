#ifndef RUNIR_KR_PS_BASE_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_BASE_CANONICALIZATION_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_data.hpp"
#include "runir/kr/ps/base/sketch_data.hpp"
#include "runir/kr/ps/canonicalization.hpp"

#include <tyr/common/canonicalization.hpp>

namespace runir::kr::ps::base
{

inline bool is_canonical(const tyr::Data<Rule>& data) noexcept
{
    return tyr::is_canonical(data.conditions) && tyr::is_canonical(data.effects);
}

inline bool is_canonical(const tyr::Data<Sketch>& data) noexcept { return tyr::is_canonical(data.rules); }

inline void canonicalize(tyr::Data<Rule>& data)
{
    tyr::canonicalize(data.conditions);
    tyr::canonicalize(data.effects);
}

inline void canonicalize(tyr::Data<Sketch>& data) { tyr::canonicalize(data.rules); }

}  // namespace runir::kr::ps::base

#endif
