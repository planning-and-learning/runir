#ifndef RUNIR_KR_DL_CNF_GRAMMAR_EXT_CONCEPT_INDEX_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_EXT_CONCEPT_INDEX_HPP_

#include "runir/kr/dl/cnf_grammar/ext/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<typename Tag>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
struct Index<runir::kr::dl::cnf_grammar::ext::Concept<Tag>> : IndexMixin<Index<runir::kr::dl::cnf_grammar::ext::Concept<Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::cnf_grammar::ext::Concept<Tag>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
