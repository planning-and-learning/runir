#ifndef RUNIR_KR_DL_SEMANTICS_EXT_DECLARATIONS_HPP_
#define RUNIR_KR_DL_SEMANTICS_EXT_DECLARATIONS_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

namespace runir::kr::dl::semantics::ext
{

using FamilyTag = runir::kr::ExtFamilyTag;

template<runir::kr::dl::CategoryTag Category>
using Denotation = runir::kr::dl::semantics::Denotation<Category>;

template<typename Index, std::unsigned_integral Block>
using IndexCoder = runir::kr::dl::semantics::IndexCoder<Index, Block>;

}  // namespace runir::kr::dl::semantics::ext

#endif
