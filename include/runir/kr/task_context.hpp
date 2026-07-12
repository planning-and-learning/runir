#ifndef RUNIR_KR_TASK_CONTEXT_HPP_
#define RUNIR_KR_TASK_CONTEXT_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

namespace runir::kr
{

template<tyr::planning::TaskKind Kind>
struct TaskContext
{
    runir::datasets::TaskSearchContextPtr<Kind> search_context;
    runir::kr::dl::semantics::Builder dl_builder;
    runir::kr::dl::semantics::DenotationRepositoryPtr dl_denotation_repository;

    TaskContext(const TaskContext&) = delete;
    TaskContext& operator=(const TaskContext&) = delete;
    TaskContext(TaskContext&&) = delete;
    TaskContext& operator=(TaskContext&&) = delete;

    static std::shared_ptr<TaskContext> create(runir::datasets::TaskSearchContextPtr<Kind> search_context)
    {
        return std::shared_ptr<TaskContext>(new TaskContext(std::move(search_context)));
    }

private:
    explicit TaskContext(runir::datasets::TaskSearchContextPtr<Kind> search_context_) :
        search_context(std::move(search_context_)),
        dl_builder(),
        dl_denotation_repository(runir::kr::dl::semantics::DenotationRepositoryFactory().create_shared())
    {
        if (!search_context)
            throw std::invalid_argument("TaskContext requires a search context.");
    }
};

template<tyr::planning::TaskKind Kind>
using TaskContextPtr = std::shared_ptr<TaskContext<Kind>>;

}  // namespace runir::kr

#endif
