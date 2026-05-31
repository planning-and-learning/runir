#ifndef RUNIR_KR_PS_BASE_SKETCH_DATA_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_DATA_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_index.hpp"
#include "runir/kr/ps/base/sketch_index.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::base::Sketch>
{
    Index<runir::kr::ps::base::Sketch> index;
    IndexList<runir::kr::ps::base::Rule> rules;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(rules);
    }

    auto cista_members() const noexcept { return std::tie(index, rules); }
    auto identifying_members() const noexcept { return std::tie(rules); }
};

}  // namespace ygg

#endif
