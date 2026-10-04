#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"

namespace runir::kr::ps::ext::detail
{

/// Borrows execution services; rule evaluators never own or refer back to an expander.
template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> Storage>
struct RuleEvaluationContext
{
    using StorageType = Storage;

    const runir::kr::TaskContextPtr<Kind>& task_context;
    Storage& storage;
    EvaluationEnvironment<Kind>& environment;
};

}  // namespace runir::kr::ps::ext::detail

#endif
