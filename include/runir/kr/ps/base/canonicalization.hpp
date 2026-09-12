#ifndef RUNIR_KR_PS_BASE_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_BASE_CANONICALIZATION_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_data.hpp"
#include "runir/kr/ps/base/sketch_data.hpp"
#include "runir/kr/ps/canonicalization.hpp"

#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::kr::ps
{

inline bool is_canonical(const ygg::Data<Rule<runir::kr::BaseFamilyTag>>& data) noexcept
{
    return ygg::is_canonical(data.conditions) && ygg::is_canonical(data.effects);
}

inline void canonicalize(ygg::Data<Rule<runir::kr::BaseFamilyTag>>& data)
{
    ygg::canonicalize(data.conditions);
    ygg::canonicalize(data.effects);
}

}  // namespace runir::kr::ps

namespace runir::kr::ps::base
{

inline bool is_canonical(const ygg::Data<Sketch>& data) noexcept { return ygg::is_canonical(data.rules); }

inline void canonicalize(ygg::Data<Sketch>& data) { ygg::canonicalize(data.rules); }

}  // namespace runir::kr::ps::base

#endif
