#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_SET_OPERATION_WORKSPACE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_SET_OPERATION_WORKSPACE_HPP_

#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include "runir/kr/dl/semantics/incremental/detail/closure.hpp"

#include <cstdint>
#include <vector>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

/// Scratch shared by sequential constructor updates; no denotation owns it.
struct SetOperationWorkspace
{
    std::vector<ygg::Index<tyr::formalism::Object>> rows;
    std::vector<std::uint8_t> marked;
    ygg::Builder<Denotation<ConceptTag>> row;
    ClosureWorkspace closure;

    void initialize(ygg::uint_t num_objects)
    {
        rows.clear();
        rows.reserve(num_objects);
        marked.assign(num_objects, 0);
        row.initialize(num_objects);
        closure.initialize(num_objects);
    }

    void mark(ygg::Index<tyr::formalism::Object> source)
    {
        const auto index = static_cast<ygg::uint_t>(source);
        if (!marked[index])
        {
            rows.push_back(source);
            marked[index] = 1;
        }
    }

    void clear_rows() noexcept
    {
        for (const auto source : rows)
            marked[static_cast<ygg::uint_t>(source)] = 0;
        rows.clear();
    }

    void mark_all()
    {
        clear_rows();
        for (ygg::uint_t source = 0; source < marked.size(); ++source)
            mark(ygg::Index<tyr::formalism::Object>(source));
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
