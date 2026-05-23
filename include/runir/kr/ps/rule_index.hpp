#ifndef RUNIR_KR_PS_RULE_INDEX_HPP_
#define RUNIR_KR_PS_RULE_INDEX_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::FamilyTag Family>
struct Index<runir::kr::ps::Rule<Family>> : IndexMixin<Index<runir::kr::ps::Rule<Family>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::Rule<Family>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
