#ifndef RUNIR_KR_TASK_CONTEXT_HPP_
#define RUNIR_KR_TASK_CONTEXT_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/execution_builder.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <memory>

namespace runir::kr
{

template<tyr::TaskKind Kind>
struct TaskContext
{
    runir::datasets::TaskSearchContextPtr<Kind> search_context;
    runir::kr::dl::BaseConstructorRepositoryPtr base_dl_repository;
    runir::kr::ps::base::RepositoryPtr base_repository;
    runir::kr::dl::ExtConstructorRepositoryPtr ext_dl_repository;
    runir::kr::ps::ext::RepositoryPtr ext_repository;
    runir::kr::dl::UnsConstructorRepositoryPtr uns_dl_repository;
    runir::kr::uns::RepositoryPtr uns_repository;
    runir::kr::dl::semantics::Builder dl_builder;
    runir::kr::dl::semantics::DenotationRepositoryPtr dl_denotation_repository;
    runir::kr::ps::ext::ExecutionBuilder<Kind> execution_builder;
    runir::kr::ps::ext::ExecutionRepositoryPtr<Kind> execution_repository;

    TaskContext(const TaskContext&) = delete;
    TaskContext& operator=(const TaskContext&) = delete;
    TaskContext(TaskContext&&) = delete;
    TaskContext& operator=(TaskContext&&) = delete;

    static std::shared_ptr<TaskContext> create(runir::datasets::TaskSearchContextPtr<Kind> search_context);

private:
    explicit TaskContext(runir::datasets::TaskSearchContextPtr<Kind> search_context);
};

extern template struct TaskContext<tyr::GroundTag>;
extern template struct TaskContext<tyr::LiftedTag>;

}  // namespace runir::kr

#endif
