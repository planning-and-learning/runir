#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DENOTATION_DELTA_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DENOTATION_DELTA_HPP_

#include "runir/kr/dl/semantics/denotation_view.hpp"

namespace runir::kr::dl::semantics::incremental
{

/// Net membership changes. Views borrow the task repository; buffers are retained.
template<ConceptOrRoleTag Category>
struct DenotationDelta
{
    DenotationElementViewList<Category> added;
    DenotationElementViewList<Category> removed;

    void clear() noexcept
    {
        added.clear();
        removed.clear();
    }
    bool empty() const noexcept { return added.empty() && removed.empty(); }
};

}  // namespace runir::kr::dl::semantics::incremental

#endif
