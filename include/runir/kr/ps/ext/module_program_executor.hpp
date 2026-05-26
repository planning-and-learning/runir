#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/graphs/declarations.hpp"
#include "runir/kr/ps/ext/module_program_proof_graph.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <tyr/planning/algorithms/brfs.hpp>
#include <tyr/planning/algorithms/iw.hpp>
#include <tyr/planning/algorithms/siw.hpp>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/successor_generator.hpp>
#include <vector>

namespace runir::kr::ps::ext
{

enum class ModuleProgramProofStatus
{
    SUCCESS,
    FAILURE,
    OUT_OF_TIME,
    OUT_OF_STATES,
};

template<tyr::planning::TaskKind Kind>
struct ModuleProgramSearchOptions
{
    tyr::planning::brfs::Options<Kind> brfs_options;
    tyr::planning::iw::Options<Kind> iw_options;
    tyr::planning::siw::Options<Kind> siw_options;
    tyr::uint_t max_arity = 0;
};

template<tyr::planning::TaskKind Kind>
struct ModuleProgramProofResults
{
    ModuleProgramProofStatus status = ModuleProgramProofStatus::SUCCESS;
    runir::datasets::TaskSearchContext<Kind> context;
    std::shared_ptr<ModuleProgramProofGraph<Kind>> graph;
    runir::graphs::EdgeIndexList deadend_transitions;
    runir::graphs::VertexIndexList open_states;
    runir::graphs::VertexIndexList cycle;

    bool is_successful() const noexcept { return status == ModuleProgramProofStatus::SUCCESS; }
};

template<tyr::planning::TaskKind Kind>
auto prove_solution(const runir::datasets::TaskSearchContext<Kind>& context,
                    ModuleProgramView program,
                    const ModuleProgramSearchOptions<Kind>& options = ModuleProgramSearchOptions<Kind>()) -> ModuleProgramProofResults<Kind>;

template<tyr::planning::TaskKind Kind>
auto find_solution(const runir::datasets::TaskSearchContext<Kind>& context,
                   ModuleProgramView program,
                   const ModuleProgramSearchOptions<Kind>& options = ModuleProgramSearchOptions<Kind>()) -> tyr::planning::SearchResult<Kind>;

#ifndef RUNIR_HEADER_INSTANTIATION
extern template auto prove_solution<tyr::planning::GroundTag>(const runir::datasets::TaskSearchContext<tyr::planning::GroundTag>& context,
                                                              ModuleProgramView program,
                                                              const ModuleProgramSearchOptions<tyr::planning::GroundTag>& options)
    -> ModuleProgramProofResults<tyr::planning::GroundTag>;

extern template auto prove_solution<tyr::planning::LiftedTag>(const runir::datasets::TaskSearchContext<tyr::planning::LiftedTag>& context,
                                                              ModuleProgramView program,
                                                              const ModuleProgramSearchOptions<tyr::planning::LiftedTag>& options)
    -> ModuleProgramProofResults<tyr::planning::LiftedTag>;

extern template auto find_solution<tyr::planning::GroundTag>(const runir::datasets::TaskSearchContext<tyr::planning::GroundTag>& context,
                                                             ModuleProgramView program,
                                                             const ModuleProgramSearchOptions<tyr::planning::GroundTag>& options)
    -> tyr::planning::SearchResult<tyr::planning::GroundTag>;

extern template auto find_solution<tyr::planning::LiftedTag>(const runir::datasets::TaskSearchContext<tyr::planning::LiftedTag>& context,
                                                             ModuleProgramView program,
                                                             const ModuleProgramSearchOptions<tyr::planning::LiftedTag>& options)
    -> tyr::planning::SearchResult<tyr::planning::LiftedTag>;
#endif

}  // namespace runir::kr::ps::ext

#include "runir/kr/ps/ext/module_program_executor_impl.hpp"

#endif
