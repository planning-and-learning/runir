#ifndef RUNIR_KR_DL_CNF_GRAMMAR_EXT_DECLARATIONS_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_EXT_DECLARATIONS_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/ext/register_index.hpp"

namespace runir::kr::dl::cnf_grammar::ext
{

using FamilyTag = runir::kr::dl::ExtFamilyTag;
using RegisterTag = runir::kr::dl::RegisterTag;

template<typename Tag>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
struct Concept;

template<runir::kr::dl::CategoryTag Category>
struct Constructor;

}  // namespace runir::kr::dl::cnf_grammar::ext

#endif
