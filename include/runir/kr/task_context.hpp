#ifndef RUNIR_KR_TASK_CONTEXT_HPP_
#define RUNIR_KR_TASK_CONTEXT_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/domain_context.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"

#include <memory>

namespace runir::kr
{

template<tyr::TaskKind Kind>
struct TaskContext
{
    DomainContextPtr domain_context;
    runir::datasets::TaskSearchContextPtr<Kind> search_context;
    runir::kr::dl::semantics::Builder dl_builder;
    runir::kr::dl::semantics::DenotationRepositoryPtr dl_denotation_repository;
    runir::kr::ps::ext::ExecutionBuilder<Kind> execution_builder;
    runir::kr::ps::ext::ExecutionRepositoryPtr<Kind> execution_repository;

    TaskContext(const TaskContext&) = delete;
    TaskContext& operator=(const TaskContext&) = delete;
    TaskContext(TaskContext&&) = delete;
    TaskContext& operator=(TaskContext&&) = delete;

    static TaskContextPtr<Kind> create(DomainContextPtr domain_context, runir::datasets::TaskSearchContextPtr<Kind> search_context);

private:
    TaskContext(DomainContextPtr domain_context, runir::datasets::TaskSearchContextPtr<Kind> search_context);
};

extern template struct TaskContext<tyr::GroundTag>;
extern template struct TaskContext<tyr::LiftedTag>;

}  // namespace runir::kr

#endif
