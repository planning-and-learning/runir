#ifndef RUNIR_KR_PS_SKETCH_DATA_HPP_
#define RUNIR_KR_PS_SKETCH_DATA_HPP_

#include "runir/kr/ps/rule_index.hpp"
#include "runir/kr/ps/sketch_index.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::ps::Sketch>
{
    Index<runir::kr::ps::Sketch> index;
    IndexList<runir::kr::ps::Rule> rules;

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(rules);
    }

    auto cista_members() const noexcept { return std::tie(index, rules); }
    auto identifying_members() const noexcept { return std::tie(rules); }
};

}  // namespace tyr

#endif
