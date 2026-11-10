#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_SET_OPERATION_WORKSPACE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_SET_OPERATION_WORKSPACE_HPP_

#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/dl/semantics/incremental/detail/closure.hpp"

#include <cstdint>
#include <vector>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

/// Scratch shared by sequential constructor updates; no denotation owns it.
struct SetOperationWorkspace
{
    MarkedRows rows { urd::layer { 0 } };
    ygg::uint_t num_objects = 0;
    urd::builder<Denotation<ConceptTag>> row;
    ClosureWorkspace closure;

    void initialize(ygg::uint_t num_objects_)
    {
        num_objects = num_objects_;
        rows.clear();
        rows.reserve(num_objects);
        row.initialize(num_objects);
        closure.initialize(num_objects);
    }

    void mark(urd::index<tyr::formalism::Object> source) { rows.insert(source); }

    [[nodiscard]] bool is_marked(urd::index<tyr::formalism::Object> source) const { return rows.contains(source); }

    void clear_rows() noexcept { rows.clear(); }

    void mark_all()
    {
        clear_rows();
        for (ygg::uint_t source = 0; source < num_objects; ++source)
            mark(urd::index<tyr::formalism::Object>(source));
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
