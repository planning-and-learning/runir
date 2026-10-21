#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_policy.hpp"

namespace runir::kr::dl::semantics::detail
{

EvaluationWorkspace& get_workspace(Builder& builder) noexcept { return builder.get_workspace(); }

ygg::database::Workspace& get_database_workspace(EvaluationWorkspace& workspace) noexcept { return workspace.get_database_workspace(); }

const tyr::formalism::planning::Repository* get_formalism_repository(const DenotationRepository& repository) noexcept
{
    return &repository.get_formalism_repository();
}

}  // namespace runir::kr::dl::semantics::detail
