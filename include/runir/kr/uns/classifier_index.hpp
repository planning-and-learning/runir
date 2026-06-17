#ifndef RUNIR_KR_UNS_CLASSIFIER_INDEX_HPP_
#define RUNIR_KR_UNS_CLASSIFIER_INDEX_HPP_

#include "runir/kr/uns/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::uns::ClassifierLiteral> : IndexMixin<Index<runir::kr::uns::ClassifierLiteral>>
{
    using Base = IndexMixin<Index<runir::kr::uns::ClassifierLiteral>>;
    using Base::Base;
};

template<>
struct Index<runir::kr::uns::ClassifierClause> : IndexMixin<Index<runir::kr::uns::ClassifierClause>>
{
    using Base = IndexMixin<Index<runir::kr::uns::ClassifierClause>>;
    using Base::Base;
};

template<>
struct Index<runir::kr::uns::Classifier> : IndexMixin<Index<runir::kr::uns::Classifier>>
{
    using Base = IndexMixin<Index<runir::kr::uns::Classifier>>;
    using Base::Base;
};

}  // namespace ygg

#endif
