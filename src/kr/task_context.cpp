#include "runir/kr/task_context.hpp"

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"

#include <stdexcept>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <utility>

namespace runir::kr
{

template<tyr::TaskKind Kind>
TaskContextPtr<Kind> TaskContext<Kind>::create(DomainContextPtr domain_context, runir::datasets::TaskSearchContextPtr<Kind> search_context)
{
    if (!domain_context)
        throw std::invalid_argument("TaskContext requires a domain context.");
    if (!search_context || !search_context->task || !search_context->execution_context || !search_context->axiom_evaluator || !search_context->state_repository
        || !search_context->successor_generator)
        throw std::invalid_argument("TaskContext requires a fully initialized search context.");
    if (domain_context->base_repository->get_dl_repository().get_planning_repository_ptr() != search_context->task->get_domain().get_repository())
        throw std::invalid_argument("TaskContext search and domain contexts must belong to the same planning domain.");
    return TaskContextPtr<Kind>(new TaskContext(std::move(domain_context), std::move(search_context)));
}

template<tyr::TaskKind Kind>
TaskContext<Kind>::TaskContext(DomainContextPtr domain_context_, runir::datasets::TaskSearchContextPtr<Kind> search_context_) :
    domain_context(std::move(domain_context_)),
    search_context(std::move(search_context_)),
    dl_builder(),
    dl_denotation_repository(runir::kr::dl::semantics::DenotationRepositoryFactory().create_shared(search_context->task->get_repository())),
    execution_builder(),
    execution_repository(runir::kr::ps::ext::ExecutionRepositoryFactory<Kind>().create_shared(search_context->state_repository,
                                                                                              dl_denotation_repository,
                                                                                              domain_context->ext_repository))
{
}

template struct TaskContext<tyr::GroundTag>;
template struct TaskContext<tyr::LiftedTag>;

}  // namespace runir::kr
