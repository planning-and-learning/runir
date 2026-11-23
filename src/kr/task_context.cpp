#include "runir/kr/task_context.hpp"

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/execution_repository.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/icp/execution_data.hpp"
#include "runir/kr/ps/icp/repository.hpp"

#include <runir/kr/database/syntax/query_repository.hpp>
#include <stdexcept>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <tyr/planning/state_builder.hpp>
#include <utility>
#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_impl.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

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
    if (domain_context->base_repository->get<runir::kr::dl::ConstructorRepository>().get_ptr<tyr::formalism::Repository>()
        != search_context->task->get_domain().get_repository())
        throw std::invalid_argument("TaskContext search and domain contexts must belong to the same planning domain.");
    return TaskContextPtr<Kind>(new TaskContext(std::move(domain_context), std::move(search_context)));
}

template<tyr::TaskKind Kind>
TaskContext<Kind>::TaskContext(DomainContextPtr domain_context_, runir::datasets::TaskSearchContextPtr<Kind> search_context_) :
    domain_context(std::move(domain_context_)),
    search_context(std::move(search_context_)),
    dl_builder(),
    dl_denotation_repository(std::make_shared<runir::kr::dl::semantics::DenotationRepository>(search_context->task->get_repository())),
    control_builder(),
    control_repository(std::make_shared<runir::kr::ps::ControlRepository<runir::kr::ExtFamilyTag>>(dl_denotation_repository, domain_context->ext_repository)),
    execution_builder(),
    execution_repository(
        std::make_shared<runir::kr::ps::ExecutionRepository<runir::kr::ExtFamilyTag, Kind>>(search_context->state_repository, control_repository)),
    icp_control_builder(),
    icp_control_repository(
        std::make_shared<runir::kr::ps::ControlRepository<runir::kr::IcpFamilyTag>>(dl_denotation_repository, domain_context->icp_repository)),
    icp_execution_builder(),
    icp_execution_repository(
        std::make_shared<runir::kr::ps::ExecutionRepository<runir::kr::IcpFamilyTag, Kind>>(search_context->state_repository, icp_control_repository))
{
}

template struct TaskContext<tyr::GroundTag>;
template struct TaskContext<tyr::LiftedTag>;

}  // namespace runir::kr

// The symbols and builders of the library's repositories, which its other sources only declare.
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::ps::ext::ControlRepository, 2)
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::ps::icp::ControlRepository, 1)
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::ps::ext::GroundExecutionRepository, 1)
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::ps::ext::LiftedExecutionRepository, 1)
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::ps::icp::GroundExecutionRepository, 1)
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::ps::icp::LiftedExecutionRepository, 1)
YGG_FORMALISM_BUILDER(, runir::kr::ps::ext::ControlBuilder, 9)
YGG_FORMALISM_BUILDER(, runir::kr::ps::icp::ControlBuilder, 8)
YGG_FORMALISM_BUILDER(, runir::kr::ps::ext::GroundExecutionBuilder, 14)
YGG_FORMALISM_BUILDER(, runir::kr::ps::ext::LiftedExecutionBuilder, 14)
YGG_FORMALISM_BUILDER(, runir::kr::ps::icp::GroundExecutionBuilder, 13)
YGG_FORMALISM_BUILDER(, runir::kr::ps::icp::LiftedExecutionBuilder, 13)
