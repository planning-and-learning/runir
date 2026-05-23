#ifndef RUNIR_KR_PS_EXT_CONDITION_DATA_HPP_
#define RUNIR_KR_PS_EXT_CONDITION_DATA_HPP_

#include "runir/kr/ps/ext/condition_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::ps::ext::ConditionVariant>
{
    Index<runir::kr::ps::ext::ConditionVariant> index;

    void clear() noexcept { tyr::clear(index); }

    auto cista_members() const noexcept { return std::tie(index); }
    auto identifying_members() const noexcept { return std::tie(); }
};

}  // namespace tyr

#endif
