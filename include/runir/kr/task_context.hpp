#ifndef RUNIR_KR_TASK_CONTEXT_HPP_
#define RUNIR_KR_TASK_CONTEXT_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/ext/execution_builder.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/uns/repository.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

namespace runir::kr
{

template<tyr::planning::TaskKind Kind>
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

    static std::shared_ptr<TaskContext> create(runir::datasets::TaskSearchContextPtr<Kind> search_context)
    {
        if (!search_context || !search_context->task || !search_context->execution_context || !search_context->axiom_evaluator
            || !search_context->state_repository || !search_context->successor_generator)
            throw std::invalid_argument("TaskContext requires a fully initialized search context.");
        return std::shared_ptr<TaskContext>(new TaskContext(std::move(search_context)));
    }

private:
    explicit TaskContext(runir::datasets::TaskSearchContextPtr<Kind> search_context_) :
        search_context(std::move(search_context_)),
        base_dl_repository(runir::kr::dl::BaseConstructorRepositoryFactory().create(search_context->task->get_repository())),
        base_repository(runir::kr::ps::base::RepositoryFactory().create(base_dl_repository)),
        ext_dl_repository(runir::kr::dl::ExtConstructorRepositoryFactory().create(search_context->task->get_repository())),
        ext_repository(runir::kr::ps::ext::RepositoryFactory().create(ext_dl_repository)),
        uns_dl_repository(runir::kr::dl::UnsConstructorRepositoryFactory().create(search_context->task->get_repository())),
        uns_repository(runir::kr::uns::RepositoryFactory().create(uns_dl_repository)),
        dl_builder(),
        dl_denotation_repository(runir::kr::dl::semantics::DenotationRepositoryFactory().create_shared()),
        execution_builder(),
        execution_repository(
            runir::kr::ps::ext::ExecutionRepositoryFactory<Kind>().create_shared(*search_context->state_repository, *dl_denotation_repository, *ext_repository))
    {
    }
};

template<tyr::planning::TaskKind Kind>
using TaskContextPtr = std::shared_ptr<TaskContext<Kind>>;

}  // namespace runir::kr

#endif
