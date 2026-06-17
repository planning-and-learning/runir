#ifndef RUNIR_KR_UNS_FEATURE_INDEX_HPP_
#define RUNIR_KR_UNS_FEATURE_INDEX_HPP_

#include "runir/kr/uns/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::uns::Feature> : IndexMixin<Index<runir::kr::uns::Feature>>
{
    using Base = IndexMixin<Index<runir::kr::uns::Feature>>;
    using Base::Base;
};

}  // namespace ygg

#endif
