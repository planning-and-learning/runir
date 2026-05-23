#ifndef RUNIR_KR_PS_BASE_SKETCH_EXECUTOR_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_EXECUTOR_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/datasets/task_class.hpp"
#include "runir/kr/ps/base/repository.hpp"

#include <memory>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/planning.hpp>

namespace runir::kr::ps::base
{

enum class SketchProofStatus
{
    SUCCESS,
    FAILURE,
    OUT_OF_TIME,
    OUT_OF_STATES,
};

template<tyr::planning::TaskKind Kind>
struct SketchProofResults
{
    SketchProofStatus status = SketchProofStatus::SUCCESS;
    runir::datasets::TaskSearchContext<Kind> context;
    std::shared_ptr<runir::datasets::StaticAnnotatedStateGraph<Kind>> graph;
    runir::graphs::EdgeIndexList deadend_transitions;
    runir::graphs::VertexIndexList open_states;
    runir::graphs::VertexIndexList cycle;

    bool is_successful() const noexcept { return status == SketchProofStatus::SUCCESS; }
};

template<tyr::planning::TaskKind Kind>
struct SketchSearchOptions
{
    tyr::planning::brfs::Options<Kind> brfs_options;
    tyr::planning::iw::Options<Kind> iw_options;
    tyr::planning::siw::Options<Kind> siw_options;
    uint_t max_arity = 0;
};

template<tyr::planning::TaskKind Kind>
auto prove_solution(const runir::datasets::TaskSearchContext<Kind>& context,
                    SketchView sketch,
                    const SketchSearchOptions<Kind>& options = SketchSearchOptions<Kind>()) -> SketchProofResults<Kind>;

template<tyr::planning::TaskKind Kind>
auto find_solution(const runir::datasets::TaskSearchContext<Kind>& context,
                   SketchView sketch,
                   const SketchSearchOptions<Kind>& options = SketchSearchOptions<Kind>()) -> tyr::planning::SearchResult<Kind>;

#ifndef RUNIR_HEADER_INSTANTIATION
extern template auto
prove_solution<tyr::planning::GroundTag>(const runir::datasets::TaskSearchContext<tyr::planning::GroundTag>& context,
                                         SketchView sketch,
                                         const SketchSearchOptions<tyr::planning::GroundTag>& options) -> SketchProofResults<tyr::planning::GroundTag>;

extern template auto
prove_solution<tyr::planning::LiftedTag>(const runir::datasets::TaskSearchContext<tyr::planning::LiftedTag>& context,
                                         SketchView sketch,
                                         const SketchSearchOptions<tyr::planning::LiftedTag>& options) -> SketchProofResults<tyr::planning::LiftedTag>;

extern template auto
find_solution<tyr::planning::GroundTag>(const runir::datasets::TaskSearchContext<tyr::planning::GroundTag>& context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::planning::GroundTag>& options) -> tyr::planning::SearchResult<tyr::planning::GroundTag>;

extern template auto
find_solution<tyr::planning::LiftedTag>(const runir::datasets::TaskSearchContext<tyr::planning::LiftedTag>& context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::planning::LiftedTag>& options) -> tyr::planning::SearchResult<tyr::planning::LiftedTag>;
#endif

}  // namespace runir::kr::ps::base

#ifdef RUNIR_HEADER_INSTANTIATION
#include "runir/kr/ps/base/sketch_executor_impl.hpp"
#endif

#endif
